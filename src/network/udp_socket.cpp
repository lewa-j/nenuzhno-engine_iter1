
#include "network.h"
#include "log.h"
#include <unistd.h>

UDPSocket::UDPSocket()
{
	handle = -1;
}

bool UDPSocket::Open(uint16_t port)
{
	//create
	handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(handle <= 0){
		LOG("Failed to create socket (%d)\n", handle);
		return false;
	}

	//bind port
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	int result = bind(handle, (sockaddr*)&address, sizeof(address));
	if(result < 0){
		LOG("Failed to bind socket (%d %d)\n", handle, result);
		return false;
	}

	//set non blocking
#ifdef WIN32
	DWORD nonBlocking = 1;
	if(ioctlsocket(handle, FIONBIO, &nonBlocking) != 0){
		LOG("Failed to set non-blocking socket (%d)\n", handle);
		return false;
	}
#else
	int nonBlocking = 1;
	if(fcntl(handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1){
		LOG("Failed to set non-blocking socket (%d)\n", handle);
		return false;
	}
#endif
	return true;
}

bool UDPSocket::isOpen()
{
	return (handle != -1);
}

void UDPSocket::Close()
{
#ifdef WIN32
	closesocket(handle);
#else
	close(handle);
#endif
	handle = -1;
}

bool UDPSocket::Send(const IPAddress &address, const char *data, int len)
{
	if(handle == -1){
		LOG("No socket\n");
		return false;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(address.ip);
	addr.sin_port = htons(address.port);

	int send_bytes = sendto(handle, data, len, 0, (sockaddr*)&addr, sizeof(addr));

	if(send_bytes != len){
		LOG("Failed to send packet to (%d.%d.%d.%d:%d): return value = %d\n", address.Get(0), address.Get(1), address.Get(2), address.Get(3), address.port, send_bytes);
		return false;
	}

	return true;
}

int UDPSocket::Receive(IPAddress &from, char *data, int len)
{
#ifdef WIN32
	typedef int socklen_t;
#endif
	sockaddr_in addr_from;
	socklen_t addr_size = sizeof(addr_from);

	int received_bytes = recvfrom(handle, data, len, 0, (sockaddr*)&addr_from, &addr_size);

	if(received_bytes <= 0)
		return 0;

	from.ip = ntohl(addr_from.sin_addr.s_addr);
	from.port = ntohs(addr_from.sin_port);

	return received_bytes;
}
