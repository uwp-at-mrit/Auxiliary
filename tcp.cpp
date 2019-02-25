#include "tcp.hpp"

using namespace WarGrey::SCADA;

/*************************************************************************************************/
bool ITCPConnection::authorized() {
	return (this->mode != TCPMode::User);
}

void ITCPConnection::set_mode(WarGrey::SCADA::TCPMode mode) {
	this->mode = mode;
}

TCPMode ITCPConnection::get_mode() {
	return this->mode;
}
