#pragma once

#include <ppltasks.h>
#include <mutex>
#include <queue>
#include <map>

#include "datum/string.hpp"
#include "datum/path.hpp"

#include "syslog.hpp"

namespace WarGrey::SCADA {
	template<class FileType, typename TypeName>
	private class IMsAppdataLogue abstract {
	public:
		virtual ~IMsAppdataLogue() noexcept {
			this->shared_task.cancel();
		}

	protected:
		virtual void on_directory_changed(Windows::Storage::StorageFolder^ rootdir) = 0;
		virtual TypeName filter_file(Platform::String^ file, Platform::String^ _ext) { return TypeName::_; }

		virtual void on_appdata(Platform::String^ file, FileType^ ftobject, TypeName type) = 0;
		virtual void on_appdata_notify(Platform::String^ file, FileType^ ftobject, TypeName type) {}
		
		virtual void on_appdata_not_found(Platform::String^ file, TypeName type) {
			this->log_message(WarGrey::SCADA::Log::Error, make_wstring(L"failed to load %s: file does not exist", file->Data()));
		}

	protected:
		virtual void log_message(WarGrey::SCADA::Log level, Platform::String^ message) {
			syslog(level, message);
		}

		virtual Windows::Foundation::IAsyncOperation<FileType^>^ read(Platform::String^ path, TypeName type) {
			return FileType::load_async(path, type);
		}

	protected:
		void cd(Platform::String^ ms_appdata_rootdir) {
			this->root = Windows::Storage::ApplicationData::Current->LocalFolder;
			this->cd_async(ms_appdata_rootdir, 0);
		}

		void list_files() {
			auto token = this->shared_task.get_token();
			auto ls = Concurrency::create_task(this->root->GetFilesAsync(), token);

			ls.then([=](Concurrency::task<Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFile^>^> list) {
				auto files = list.get();
				
				for (unsigned int idx = 0; idx < files->Size; idx++) {
					Windows::Storage::StorageFile^ file = files->GetAt(idx);
					Platform::String^ filename = file_name_from_path(file->Path);
					Platform::String^ file_ext = file_extension_from_path(file->Path);
					TypeName filetype = this->filter_file(filename, file_ext);
					
					if (filetype != TypeName::_) {
						this->load_file(filename, filetype);
					}
				}
			}).then([=](Concurrency::task<void> maybe_exn) {
				try {
					maybe_exn.get();
				} catch (Platform::Exception^ e) {
					this->log_message(WarGrey::SCADA::Log::Error,
						make_wstring(L"failed to list the directory %s: %s",
							this->root->Path->Data(), e->Message->Data()));
				} catch (Concurrency::task_canceled&) {
					this->log_message(WarGrey::SCADA::Log::Debug, make_wstring(L"cancelled traversing %s", this->root->Path->Data()));
				} catch (std::exception& e) {
					this->log_message(WarGrey::SCADA::Log::Debug, make_wstring(L"list_files: unexcepted exception: %s", e.what()));
				}
			});
		}
		
		void load_file(Platform::String^ filename, TypeName filetype) {
			this->load_async(filename, filetype);
		}

	private:
		void cd_async(Platform::String^ path, unsigned int search_start = 0) {
			if (search_start < path->Length()) {
				int slash_pos = path_next_slash_position(path, search_start);
				Platform::String^ subdir = substring(path, search_start, slash_pos);
				unsigned int subsize = subdir->Length();
				unsigned int next_search_pos = search_start + subdir->Length() + 1;
				
				if ((subsize == 0) || (subdir->Equals("."))) { // TODO: deal with '..'
					this->cd_async(path, next_search_pos);
				} else {
					auto token = this->shared_task.get_token();
					auto cd = Concurrency::create_task(this->root->CreateFolderAsync(subdir, Windows::Storage::CreationCollisionOption::OpenIfExists), token);

					cd.then([=](Concurrency::task<Windows::Storage::StorageFolder^> sdir) {
						this->root = sdir.get();
						this->cd_async(path, next_search_pos);
					}).then([=](Concurrency::task<void> maybe_exn) {
						try {
							maybe_exn.get();
						} catch (Platform::Exception^ e) {
							this->log_message(WarGrey::SCADA::Log::Error,
								make_wstring(L"failed to changed to directory %s: %s",
									path->Data(), e->Message->Data()));
						} catch (Concurrency::task_canceled&) {
							this->log_message(WarGrey::SCADA::Log::Debug, make_wstring(L"cancelled traversing %s", path->Data()));
						} catch (std::exception& e) {
							this->log_message(WarGrey::SCADA::Log::Debug, make_wstring(L"cd: unexcepted exception: %s", e.what()));
						}
					});
				}
			} else {
				this->on_directory_changed(this->root);
			}
		}

		void load_async(Platform::String^ filename, TypeName filetype) {
			auto token = this->shared_task.get_token();
			auto get_file = Concurrency::create_task(this->root->GetFileAsync(filename), token);

			get_file.then([=](Concurrency::task<Windows::Storage::StorageFile^> sfile) {
				Windows::Storage::StorageFile^ file = sfile.get(); // Stupid Microsoft: `sfile.get()` seems lost itself if the file does not exist.

				this->log_message(Log::Debug, make_wstring(L"loading %s", file->Path->Data()));

				return Concurrency::create_task(this->read(file->Path, filetype), token);
			}).then([=](Concurrency::task<FileType^> doc) {
				try {
					FileType^ ftobject = doc.get();

					if (ftobject != nullptr) {
						this->log_message(Log::Debug, make_wstring(L"loaded %s", filename->ToString()->Data()));

						this->on_appdata(filename, ftobject, filetype);
						this->on_appdata_notify(filename, ftobject, filetype);
					} else {
						this->on_appdata_not_found(filename, filetype);
					}
				} catch (Platform::Exception^ e) {
					switch (e->HResult) {
					case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND): {
						this->on_appdata_not_found(filename, filetype);
					}; break;
					default: {
						this->log_message(WarGrey::SCADA::Log::Error,
							make_wstring(L"failed to load %s: %s",
								filename->ToString()->Data(), e->Message->Data()));
					}
					}
				} catch (Concurrency::task_canceled&) {
					this->log_message(WarGrey::SCADA::Log::Debug,
						make_wstring(L"cancelled loading %s", filename->ToString()->Data()));
				} catch (std::exception& e) {
					this->log_message(WarGrey::SCADA::Log::Debug,
						make_wstring(L"load: unexcepted exception: %s", e.what()));
				}
			});
		}

	private:
		Concurrency::cancellation_token_source shared_task;
		Windows::Storage::StorageFolder^ root;
	};
}
