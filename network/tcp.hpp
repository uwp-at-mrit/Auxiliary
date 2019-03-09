#pragma once

#include "syslog.hpp"

namespace WarGrey::SCADA {
	private enum class TCPMode { Root, User, Debug, _ };

	private class ITCPConnection abstract {
	public:
		virtual Platform::String^ device_hostname() = 0;
		virtual Platform::String^ device_description() = 0;

	public:
		virtual Syslog* get_logger() = 0;
		virtual void shake_hands() = 0;
		virtual bool connected() = 0;
		virtual void suicide() = 0;

	public:
		virtual void send_scheduled_request(long long count, long long interval, long long uptime) = 0;

	public:
		bool authorized();
		void set_mode(WarGrey::SCADA::TCPMode mode);
		WarGrey::SCADA::TCPMode get_mode();

	private:
		WarGrey::SCADA::TCPMode mode = TCPMode::Root;
	};
}
