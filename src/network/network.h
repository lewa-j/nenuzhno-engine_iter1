
#pragma once

#ifdef WIN32
	#include <winsock2.h>
#else//unix
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
#endif

#include <stdint.h>

#define PROTOCOL_ID 0x14881337

struct IPAddress
{
	uint32_t ip;
	uint16_t port;

	IPAddress(){ip=0;port=0;}
	IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t p);
	uint8_t Get(int i) const;
	void Set(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t p);
};

#include "network/udp_socket.h"

typedef void (* SerializeFun)(char *, int *);

//TODO: clients list
class NetworkManager
{
	bool initialized;
	UDPSocket serverSocket;
	IPAddress serverAddress;
	UDPSocket clientSocket;
	int clientId;
	SerializeFun Serialize;
	SerializeFun Deserialize;
	float fromLastClientUpdate;
public:
	NetworkManager();

	bool Init();
	void Shutdown();
	bool HostServer(uint16_t port);
	void UpdateServer(float deltaTime);
	bool ConnectToServer(IPAddress address, uint16_t clientPort);
	void UpdateClient(float deltaTime);

	void SetSerializeCallbacks(SerializeFun ser, SerializeFun deser);
};
