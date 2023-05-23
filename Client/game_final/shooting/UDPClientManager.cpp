#include "UDPClientManager.h"




UDPClientManager::Status UDPClientManager::Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
	sf::Socket::Status status;
	PacketInfo packetInfo = PacketInfo{ packetCount, packet, std::chrono::system_clock::now(), std::chrono::system_clock::now(), ip, port };
	packet << packetCount;
	packetMap[packetCount++] = packetInfo;
	status = _socket.send(packet, ip, port);
	return Status();
}

void UDPClientManager::Bind()
{
	sf::Socket::Status status = _socket.bind(_port);
 
	while (status != sf::Socket::Status::Done)
	{
		_port++;
		status = _socket.bind(_port);
	}

}

void UDPClientManager::TryConnection(sf::String user)
{
	sf::Packet tryConnectionPacket;
	tryConnectionPacket << (int)PacketType::TRYCONNECTION;
	tryConnectionPacket << user;
	std::cout << "Trying Connection (client)" << std::endl;
	//tryConnectionPacket << _port;
	Send(tryConnectionPacket, sf::IpAddress("127.0.0.1"), 5000);
}
