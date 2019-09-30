#pragma once

#include <ppltasks.h>
#include <mutex>
#include <queue>
#include <map>

#include "datum/string.hpp"

#include "syslog.hpp"

namespace WarGrey::SCADA {
	template<class FileType, typename Hint>
	private class IMsAppdata abstract {
	public:
		virtual ~IMsAppdata() noexcept {
			this->shared_task.cancel();
		}

	protected:
		virtual void on_appdata(Windows::Foundation::Uri^ ms_appdata, FileType^ ftobject, Hint hint) = 0;

		virtual void on_appdata_notify(Windows::Foundation::Uri^ ms_appdata, FileType^ ftobject, Hint hint) {}
		
		virtual void on_appdata_not_found(Windows::Foundation::Uri^ ms_appdata, Hint hint) {
			this->log_message(WarGrey::SCADA::Log::Error,
				make_wstring(L"failed to load %s: file does not exist",
					ms_appdata->ToString()->Data()));
		}

	protected:
		virtual void log_message(WarGrey::SCADA::Log level, Platform::String^ message) {
			syslog(level, message);
		}

		virtual Windows::Foundation::IAsyncOperation<FileType^>^ read(Platform::String^ path) {
			return FileType::load_async(path);
		}

	protected:
		void load(Windows::Foundation::Uri^ ms_appdata, Hint hint, Platform::String^ file_type = "matrix data") {
			auto uuid = ms_appdata->ToString()->GetHashCode();

			IMsAppdata<FileType, Hint>::critical_sections[uuid].lock();
			auto reference = IMsAppdata<FileType, Hint>::refcounts.find(uuid);

			if (reference == IMsAppdata<FileType, Hint>::refcounts.end()) {
				IMsAppdata<FileType, Hint>::refcounts[uuid] = 0;
				IMsAppdata<FileType, Hint>::queues[uuid].push(this);
				this->load_async(uuid, ms_appdata, hint, file_type);
			} else if (reference->second > 0) {
				reference->second += 1;
				this->on_appdata(ms_appdata, IMsAppdata<FileType, Hint>::filesystem[uuid], hint);

				this->log_message(Log::Debug, 
					make_wstring(L"reused the %s: %s with reference count %d",
						file_type->Data(), ms_appdata->ToString()->Data(),
						IMsAppdata<FileType, Hint>::refcounts[uuid]));
			} else {
				IMsAppdata<FileType, Hint>::queues[uuid].push(this);
				this->log_message(Log::Debug,
					make_wstring(L"waiting for the %s: %s",
						file_type->Data(), ms_appdata->ToString()->Data()));
			}
			IMsAppdata<FileType, Hint>::critical_sections[uuid].unlock();
		}

		void unload(Windows::Foundation::Uri^ ms_appdata) {
			auto uuid = ms_appdata->ToString()->GetHashCode();

			IMsAppdata<FileType, Hint>::critical_sections[uuid].lock();
			auto reference = IMsAppdata<FileType, Hint>::refcounts.find(uuid);

			if (reference != IMsAppdata<FileType, Hint>::refcounts.end()) {
				if (reference->second <= 1) {
					IMsAppdata<FileType, Hint>::refcounts.erase(uuid);
					IMsAppdata<FileType, Hint>::filesystem.erase(uuid);
				} else {
					reference->second -= 1;
				}
			}
			IMsAppdata<FileType, Hint>::critical_sections[uuid].unlock();
		}

	private:
		void load_async(int uuid, Windows::Foundation::Uri^ ms_appdata, Hint hint, Platform::String^ file_type) {
			auto token = this->shared_task.get_token();
			auto get_file = Concurrency::create_task(Windows::Storage::StorageFile::GetFileFromApplicationUriAsync(ms_appdata), token);

			get_file.then([=](Concurrency::task<Windows::Storage::StorageFile^> sfile) {
				Windows::Storage::StorageFile^ file = sfile.get(); // Stupid Microsoft: `sfile.get()` seems lost itself if the file does not exist.

				this->log_message(Log::Debug,
					make_wstring(L"loading the %s: %s",
						file_type->Data(), ms_appdata->ToString()->Data()));

				return Concurrency::create_task(this->read(file->Path), token);
			}).then([=](Concurrency::task<FileType^> doc) {
				IMsAppdata<FileType, Hint>::critical_sections[uuid].lock();
				
				try {
					FileType^ ftobject = doc.get();

					if (ftobject != nullptr) {
						std::queue<IMsAppdata<FileType, Hint>*> q = IMsAppdata<FileType, Hint>::queues[uuid];

						IMsAppdata<FileType, Hint>::filesystem[uuid] = ftobject;

						this->log_message(Log::Debug,
							make_wstring(L"loaded the %s: %s",
								file_type->Data(), ms_appdata->ToString()->Data()));

						while (!q.empty()) {
							auto self = q.front();

							self->on_appdata(ms_appdata, ftobject, hint);
							self->on_appdata_notify(ms_appdata, ftobject, hint);

							IMsAppdata<FileType, Hint>::refcounts[uuid] += 1;
							q.pop();
						}

						this->log_message(Log::Debug,
							make_wstring(L"loaded the %s: %s with reference count %d",
								file_type->Data(), ms_appdata->ToString()->Data(),
								IMsAppdata<FileType, Hint>::refcounts[uuid]));
						IMsAppdata<FileType, Hint>::queues.erase(uuid);
					} else {
						this->on_appdata_not_found(ms_appdata, hint);
					}
				} catch (Platform::Exception^ e) {
					IMsAppdata<FileType, Hint>::clear(uuid);

					switch (e->HResult) {
					case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND): {
						this->on_appdata_not_found(ms_appdata, hint);
					}; break;
					default: {
						this->log_message(WarGrey::SCADA::Log::Error,
							make_wstring(L"failed to load %s: %s",
								ms_appdata->ToString()->Data(), e->Message->Data()));
					}
					}
				} catch (Concurrency::task_canceled&) {
					IMsAppdata<FileType, Hint>::clear(uuid);
					this->log_message(WarGrey::SCADA::Log::Debug,
						make_wstring(L"cancelled loading %s", ms_appdata->ToString()->Data()));
				} catch (std::exception& e) {
					IMsAppdata<FileType, Hint>::clear(uuid);
					this->log_message(WarGrey::SCADA::Log::Debug,
						make_wstring(L"unexcepted exception: %s", e.what()));
				}

				IMsAppdata<FileType, Hint>::critical_sections[uuid].unlock();
			});
		}

	private:
		static void clear(int uuid) {
			std::queue<IMsAppdata<FileType, Hint>*> q = IMsAppdata<FileType, Hint>::queues[uuid];

			while (!q.empty()) {
				q.pop();
			}

			IMsAppdata<FileType, Hint>::refcounts.erase(uuid);
			IMsAppdata<FileType, Hint>::queues.erase(uuid);
		}

	private:
		Concurrency::cancellation_token_source shared_task;
		
	private:
		static std::map<int, size_t> refcounts;
		static std::map<int, FileType^> filesystem;
		static std::map<int, std::queue<IMsAppdata<FileType, Hint>*>> queues;
		static std::map<int, std::mutex> critical_sections;
	};

	template<class FileType, typename Hint> std::map<int, size_t> IMsAppdata<FileType, Hint>::refcounts;
	template<class FileType, typename Hint> std::map<int, FileType^> IMsAppdata<FileType, Hint>::filesystem;
	template<class FileType, typename Hint> std::map<int, std::queue<IMsAppdata<FileType, Hint>*>> IMsAppdata<FileType, Hint>::queues;
	template<class FileType, typename Hint> std::map<int, std::mutex> IMsAppdata<FileType, Hint>::critical_sections;
}
