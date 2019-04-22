#pragma once

#include <list>

#include "syslog.hpp"

#include "datum/time.hpp"

namespace WarGrey::SCADA {
	private enum class TCPMode { Root, User, Debug, _ };
	
	class ITCPStatusListener;

	private class ITCPConnection abstract {
	public:
		virtual ~ITCPConnection() noexcept;

	public:
		virtual Platform::String^ device_hostname() = 0;
		virtual Platform::String^ device_description() = 0;

	public:
		virtual Syslog* get_logger() = 0;
		virtual void shake_hands() = 0;
		virtual bool connected() = 0;
		virtual void suicide() = 0;
		virtual void suicide_if_timeout(long long ms) = 0;

	public:
		virtual void send_scheduled_request(long long count, long long interval, long long uptime) = 0;
		
	public:
		bool authorized();
		void set_mode(WarGrey::SCADA::TCPMode mode);
		WarGrey::SCADA::TCPMode get_mode();

	public:
		void set_suicide_timeout(long long timeout_ms);

	protected:
		virtual void reset_heartbeat() = 0;

	private:
		WarGrey::SCADA::TCPMode mode = TCPMode::Root;
		Platform::Object^ killer;
	};

	private class ITCPStatusListener {
	public:
		virtual void on_connectivity_changed(WarGrey::SCADA::ITCPConnection* master, bool connected) {}

	public:
		virtual void on_send_data(WarGrey::SCADA::ITCPConnection* master, long long bytes, double span_ms, double timestamp_ms) {}
		virtual void on_receive_data(WarGrey::SCADA::ITCPConnection* master, long long bytes, double span_ms, double timestamp_ms) {}
	};

	template<class TCPStatusListener>
	private class ITCPFeedBackConnection abstract : public WarGrey::SCADA::ITCPConnection {
	public:
		void push_status_listener(TCPStatusListener* listener) {
			if (listener != nullptr) {
				this->listeners.push_back(listener);
				listener->on_connectivity_changed(this, this->connected());
			}
		}

	public:
		void notify_connectivity_changed() {
			bool connected = this->connected();

			for (auto it = this->listeners.begin(); it != this->listeners.end(); it++) {
				(*it)->on_connectivity_changed(this, connected);
			}
		}

		void notify_data_sent(long long bytes, double span_ms) {
			double timestamp = current_inexact_milliseconds();

			for (auto it = this->listeners.begin(); it != this->listeners.end(); it++) {
				(*it)->on_send_data(this, bytes, span_ms, timestamp);
			}
		}

		void notify_data_received(long long bytes, double span_ms) {
			double timestamp = current_inexact_milliseconds();

			for (auto it = this->listeners.begin(); it != this->listeners.end(); it++) {
				(*it)->on_receive_data(this, bytes, span_ms, timestamp);
			}

			this->last_heartbeat = (long long)(std::round(timestamp));
		}

	public:
		void suicide_if_timeout(long long timeout) override {
			long long now = current_milliseconds();

			if ((now - this->last_heartbeat) > timeout) {
				this->suicide();
				this->reset_heartbeat();
			}
		}

	protected:
		void reset_heartbeat() override {
			this->last_heartbeat = current_milliseconds();
		}

	protected:
		std::list<TCPStatusListener*> listeners;

	private:
		long long last_heartbeat;
	};
}
