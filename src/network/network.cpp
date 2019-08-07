
#include <vec3.hpp>
#include "network.h"
#include "log.h"

IPAddress::IPAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t p)
{
	Set(a,b,c,d,p);
}

uint8_t IPAddress::Get(int i) const
{
	return ((uint8_t *)&ip)[3-i];
}

void IPAddress::Set(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t p)
{
	ip = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
	port = p;
}

NetworkManager::NetworkManager()
{
	initialized = false;
	Serialize = 0;
	Deserialize = 0;
	clientId = 0;
	fromLastClientUpdate = 0;
}

bool NetworkManager::Init()
{
	if(initialized)
		return true;

	Log("Init network\n");
	initialized = true;
#ifdef WIN32
	WSAData data;
	return WSAStartup(MAKEWORD(2,2), &data) == NO_ERROR;
#else
	return true;
#endif
}

void NetworkManager::Shutdown()
{
#ifdef WIN32
	WSACleanup();
#endif
	initialized = false;
}

bool NetworkManager::HostServer(uint16_t port)
{
	if(!serverSocket.Open(port)){
		Log("Can't open server socket\n");
		return false;
	}
	Log("Open server socket %d\n",port);
	return true;
}

void NetworkManager::UpdateServer(float deltaTime)
{
	if(!initialized)
		return;

	if(!serverSocket.isOpen())
		return;

	while(true){
		char packet_data[256];
		uint32_t max_packet_size = sizeof(packet_data);

		IPAddress from_addr;
		int rec_bytes = serverSocket.Receive(from_addr, packet_data, max_packet_size);

		if(!rec_bytes)
			break;

		//LOG("Received packet from %d.%d.%d.%d:%d\n%s\n", from_addr.Get(3), from_addr.Get(2), from_addr.Get(1), from_addr.Get(0), from_addr.port, packet_data);

		if(*(uint32_t*)packet_data != PROTOCOL_ID)
			continue;

		if(packet_data[4]=='c' && packet_data[5]=='r'){
			Log("Got connect request\n");

			char request[8];
			*(uint32_t*)request = PROTOCOL_ID;
			request[4] = 'c';
			request[5] = 's';
			request[6] = 1;//client_id
			request[7] = 0;

			if(!serverSocket.Send(from_addr, request, 8)){
				Log("Can't send connect success message\n");
			}
		}

		if(packet_data[4] == 'u')//update
		{
			if(Deserialize){
				int message_size = rec_bytes-5;
				Deserialize(packet_data+5, &message_size);
			}
		}
	}
}

bool NetworkManager::ConnectToServer(IPAddress address, uint16_t clientPort)
{
	if(!initialized)
		return false;

	if(!clientSocket.isOpen()){
		if(!clientSocket.Open(clientPort)){
			Log("Can't open client socket\n");
			return false;
		}
		Log("Open client socker %d\n",clientPort);
	}

	char request[8];
	*(uint32_t*)request = PROTOCOL_ID;
	request[4]='c';
	request[5]='r';
	request[6]=0;
	request[7]=0;

	Log("Try connect to %d.%d.%d.%d:%d\n",address.Get(0),address.Get(1),address.Get(2),address.Get(3),address.port);
	return clientSocket.Send(address, request, 8);

	//return true;
}

void NetworkManager::UpdateClient(float deltaTime)
{
	if(!initialized)
		return;

	if(!clientSocket.isOpen()){
		LOG("UpdateClient clientSocket closed!\n");
		return;
	}

	//receive
	while(true)
	{
		char packet_data[256];
		uint32_t max_packet_size = sizeof(packet_data);

		IPAddress from_addr;
		int rec_bytes = clientSocket.Receive(from_addr, packet_data, max_packet_size);

		if(!rec_bytes)
			break;

		LOG("Received packet from %d.%d.%d.%d:%d\n%s\n", from_addr.Get(3), from_addr.Get(2), from_addr.Get(1), from_addr.Get(0), from_addr.port, packet_data);

		if(*(uint32_t*)packet_data != PROTOCOL_ID)
			continue;

		if(packet_data[4]=='c' && packet_data[5]=='s')
		{
			serverAddress = from_addr;
			clientId = packet_data[6];
			LOG("Connect success (%d)\n", clientId);
		}
	}

	//send
	if(clientId && Serialize)
	{
		fromLastClientUpdate += deltaTime;
		if(fromLastClientUpdate>(1.0f/30))
		{
			fromLastClientUpdate=0;
			char packet[256];
			int size = 0;//empty
			*(uint32_t*)packet = PROTOCOL_ID;
			packet[4] = 'u';
			packet[5] = clientId;
			Serialize(packet+6, &size);

			clientSocket.Send(serverAddress, packet, 6+size);
		}
	}
}

void NetworkManager::SetSerializeCallbacks(SerializeFun ser, SerializeFun deser)
{
	Serialize = ser;
	Deserialize = deser;
}
