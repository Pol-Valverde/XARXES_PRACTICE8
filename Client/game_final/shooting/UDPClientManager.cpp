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

void UDPClientManager::Receive()
{
	sf::Packet packet;
	sf::IpAddress remoteIp;
	unsigned short remotePort;
	while (true)
	{
		sf::Socket::Status status = _socket.receive(packet, remoteIp, remotePort);
		sf::String user;
		if (status == sf::Socket::Done)
		{
			int type;
			packet >> type;//sempre rebem primer el type
			switch ((PacketType)type)
			{
				case PacketType::CHALLENGE:
				{
					isChallenge = true;
					
					packet >> challengeNumber1;
					packet >> challengeNumber2;
					sf::Packet challengePacket;
					std::cout << "solve this math operation: " << challengeNumber1 << "+" << challengeNumber2 << std::endl;
					break;
				}

				case PacketType::CANCONNECT:
				{
					packet >> id;
					isChallenge = false;
					std::cout << "CAN Connect id:" << id<< std::endl;
					selectMatchMakingOption = true;

					break;
				}
				case PacketType::CANNOTCONNECT:
				{
					isChallenge = true;

					
					packet >> challengeNumber1;
					packet >> challengeNumber2;
					sf::Packet challengePacket;
					std::cout << "solve this math operation: " << challengeNumber1 << "+" << challengeNumber2 << std::endl;
					break;
				}
				
			}
		}
	}
}

void UDPClientManager::SendChallenge(int result)
{
	sf::Packet tryChallenge;
	std::cout << result;
	tryChallenge << (int)PacketType::CHALLENGE;
	tryChallenge << result;
	//tryConnectionPacket << _port;
	Send(tryChallenge, sf::IpAddress("127.0.0.1"), 5000);

}

void UDPClientManager::SendSelectMatchMakingType(int result)
{

	selectMatchMakingOption = false;
	_startPlaying = true;
	
	sf::Packet matchMakingPacket;
	matchMakingPacket << (int)PacketType::MATCHMAKINGMODE;
	matchMakingPacket << result;
	Send(matchMakingPacket, sf::IpAddress("127.0.0.1"), 5000);
}
