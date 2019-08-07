
#pragma once

#include <stdint.h>

class UDPSocket
{
	int handle;
public:
	UDPSocket();

	bool Open(uint16_t port);
	bool isOpen();
	void Close();
	bool Send(const IPAddress &address, const char *data, int len);
	int Receive(IPAddress &from, char *data, int len);
};
