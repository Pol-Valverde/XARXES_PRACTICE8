#include "UDPClientManager.h"

#define PKT_LOSS_PROB 5// 5 sobre 1000 -> 0.5%
#define PACKET_TIMEOUT_IN_MILLIS 500 // milliseconds

UDPClientManager::Status UDPClientManager::Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
	sf::Socket::Status status;
	PacketInfo packetInfo = PacketInfo{ packetCount, packet, std::chrono::system_clock::now(), std::chrono::system_clock::now(), ip, port };
	packet << packetCount;
	packetMap[packetCount++] = packetInfo;
	int probabilty = probLossManager.generate_prob();
	if (probabilty > PKT_LOSS_PROB)
	{
		status = _socket.send(packet, ip, port);
	}
	return Status();
}

UDPClientManager::Status UDPClientManager::SendNonCritical(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
	sf::Socket::Status status;

	status = _socket.send(packet, ip, port);

	return Status();
}

UDPClientManager::Status UDPClientManager::ReSend(sf::Packet& packet, int packetId, sf::IpAddress ip, unsigned short port)
{
	std::cout << "RESEND" << std::endl;

	// Fill packet with data:
	sf::Socket::Status status;
	PacketInfo packetInfo = PacketInfo{ packetId, packet, std::chrono::system_clock::now(), ip, port };

	packetMap[packetId] = packetInfo;
	int probabilty = probLossManager.generate_prob();
	if (probabilty > PKT_LOSS_PROB)
	{
		status = _socket.send(packet, ip, port);
	}
	packet.clear();

	Status tempStatus;

	switch (status)
	{
	case sf::Socket::Done:
		tempStatus = Status::Done;
		break;

	case sf::Socket::NotReady:
		std::cout << "Error: NotReady.";
		tempStatus = Status::Error; // Since we do not know how sf:Socket::Status works, we return Error directly.
		break;

	case sf::Socket::Partial:
		std::cout << "Error: Partial.";
		tempStatus = Status::Error; // Since we do not know how sf:Socket::Status works, we return Error directly.
		break;

	case sf::Socket::Disconnected: // Per com hem fet el codi, retornar Status::Disconnected fa que el servidor es tanqui.
								   // per això ho canviem per Status::Error.
		std::cout << "Error: cliente desconectado.";
		tempStatus = Status::Error;
		break;

	case sf::Socket::Error:
		std::cout << "Error: al enviar paquete.";
		tempStatus = Status::Error;
		break;

	default:
		std::cout << "Error: default";
		tempStatus = Status::Error; // Since we do not know how sf:Socket::Status works, we return Error directly.
		break;
	}

	// Return:
	return tempStatus;
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
	SendNonCritical(tryConnectionPacket, sf::IpAddress("127.0.0.1"), 5000);
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

					int packetId;
					packet >> packetId;

					sf::Packet challengePacket;
					std::cout << "solve this math operation: " << challengeNumber1 << "+" << challengeNumber2 << std::endl;

					SendACKToServer(sf::IpAddress("127.0.0.1"), 5000, packetId);

					break;
				}

				case PacketType::CANCONNECT:
				{
					packet >> id;
					isChallenge = false;
					std::cout << "CAN Connect id:" << id<< std::endl;
					selectMatchMakingOption = true;
					int id;
					packet >> id;
					SendACKToServer(remoteIp, remotePort, id);
					break;
				}
				case PacketType::CANNOTCONNECT:
				{
					isChallenge = true;

					
					packet >> challengeNumber1;
					packet >> challengeNumber2;
					sf::Packet challengePacket;
					std::cout << "solve this math operation: " << challengeNumber1 << "+" << challengeNumber2 << std::endl;
					int id;
					packet >> id;
					SendACKToServer(remoteIp, remotePort, id);
					break;
				}
				case PacketType::DISCONNECT:
				{
					Disconnect();
					break;
				}
				case PacketType::PING:
				{
					sf::Packet pongPacket;
					pongPacket << (int)PacketType::PONG;
					pongPacket << id;
					//Send(pongPacket, sf::IpAddress("127.0.0.1"), 5000);
				}
				case PacketType::ACK:
				{
					int tempID;
					packet >> tempID;
					std::cout << "recieved ack from server" << std::endl;
					packetsToDelete.push_back(tempID);
					break;
				}
				
			}
		}
	}
}

void UDPClientManager::Disconnect()
{
	_socket.unbind();
	exit(0);
	std::cout << "disconecting" << std::endl;
}

void UDPClientManager::CheckTimeStamp()
{
	while (true)
	{
		if (packetMap.size() > 0) {
			//sstd::cout << "checking time client" << std::endl;

			auto current_time = std::chrono::system_clock::now();

			for (auto packet : packetMap)
			{
				//check if we need to resend the packet
				if ((current_time - packet.second.timeSent) > std::chrono::milliseconds(PACKET_TIMEOUT_IN_MILLIS))
				{
					std::cout << "resending message" << std::endl;

					ReSend(packet.second.packet, packet.first, sf::IpAddress("127.0.0.1"), 5000); // ADDED THIS!
					packet.second.timeSent = std::chrono::system_clock::now();
				}
			}
		}

		//delete confirmed packet
		if (packetsToDelete.size() > 0)
		{
			for (int id : packetsToDelete)
			{
				packetMap.erase(id);
			}
			packetsToDelete.clear();
		}
	}
}

void UDPClientManager::SendACKToServer(sf::IpAddress remoteIP, unsigned short remotePort, int id)
{
	sf::Packet ACKpacket;
	sf::Socket::Status status;

	ACKpacket << (int)PacketType::ACK;
	ACKpacket << id;
	int probabilty = probLossManager.generate_prob();
	if (probabilty > PKT_LOSS_PROB)
	{
		status = _socket.send(ACKpacket, remoteIP, remotePort);
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
