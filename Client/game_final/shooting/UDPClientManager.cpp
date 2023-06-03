#include "UDPClientManager.h"

#define PKT_LOSS_PROB 5// 5 sobre 1000 -> 0.5%
#define PACKET_TIMEOUT_IN_MILLIS 500 // milliseconds

UDPClientManager::UDPClientManager(unsigned short port, sf::IpAddress ip) :_port(port), _ip(ip)
{
	_socket.setBlocking(true);
}


UDPClientManager::Status UDPClientManager::Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
	packetCount++;
	sf::Socket::Status status;
	PacketInfo packetInfo = PacketInfo{ packetCount, packet, std::chrono::system_clock::now(), std::chrono::system_clock::now(), ip, port };
	packet << packetCount;

	packetMap[packetCount] = packetInfo;
	
	int probabilty = probLossManager.generate_prob();
	if (probabilty > packetLossProb)
	{
		std::cout << "PacketCount : " << packetCount<<std::endl;
		status = _socket.send(packet, ip, port);
	}
	packet.clear();
	return Status();
}

UDPClientManager::Status UDPClientManager::SendNonCritical(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
	sf::Socket::Status status;

	status = _socket.send(packet, ip, port);
	packet.clear();
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
	if (probabilty > packetLossProb)
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
	_username = user;
	int tempId = -1;
	sf::Packet tryConnectionPacket;
	tryConnectionPacket << tempId;
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
			std::cout << type;
			switch ((PacketType)type)
			{
				case PacketType::CHALLENGE:
				{
					if (_setId)
					{
						_setId = false;

						int clientId;
						packet >> clientId;
						_client.id = clientId;
					}

					isChallenge = true;

					packet >> challengeNumber1;
					packet >> challengeNumber2;

					int packetId;
					packet >> packetId;

					sf::Packet challengePacket;
					std::cout << "solve this math operation: " << challengeNumber1 << "+" << challengeNumber2 << std::endl;
					// Això no es printa. Hi ha quelcom que té prioritat i es printa per sobre (crec que la 108 de game.cpp).

					SendACKToServer(sf::IpAddress("127.0.0.1"), 5000, packetId);

					break;
				}

				case PacketType::CANCONNECT:
				{
					int clientId;
					packet >> clientId;
					_client = Client(_username, clientId);
					isChallenge = false;
					std::cout << "CAN Connect id:" << id << std::endl;
					selectMatchMakingOption = true;
					int packetId;
					packet >> packetId;
					SendACKToServer(remoteIp, remotePort, packetId);
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
					std::cout << "Ping Recived :)" << std::endl;
					sf::Packet pongPacket;
					pongPacket << _client.id;
					pongPacket << (int)PacketType::PONG;

					SendNonCritical(pongPacket, sf::IpAddress("127.0.0.1"), 5000);
				}
				case PacketType::ACK:
				{
					int tempID;
					packet >> tempID;
					std::cout << "recieved ack from server         -->" <<tempID << std::endl;
					packetsToDelete.push_back(tempID);
					break;
				}
				case PacketType::MOVEMENT:
				{
					
					packet >> _client.posX;
					packet >> _client.posY;
					UpdatePosition = true;
					break;
				}
				case PacketType::RIVALMOVEMENT:
				{

					packet >> _client.rivalPosX;
					packet >> _client.rivalPosY;
					UpdateRivalPosition = true;
					break;
				}
				case PacketType::PLAYERNUMBER:
				{
					int playerNum;
					packet >> playerNum;
					int packetID;
					packet >> packetID;
					SendACKToServer(sf::IpAddress("127.0.0.1"), 5000,packetID);

					if (playerNum == 1) {
						isPlayerOne = true;
						_client.rivalPosX = 40;
						_client.rivalPosY = 40;
					}
					else {
						isPlayerOne = false;
						_client.posX = 40;
						_client.posY = 40;
					}
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
			
		}
	}
}

void UDPClientManager::SendACKToServer(sf::IpAddress remoteIP, unsigned short remotePort, int id)
{
	sf::Packet ACKpacket;
	sf::Socket::Status status;
	ACKpacket << _client.id;
	ACKpacket << (int)PacketType::ACK;
	ACKpacket << id;

	int probabilty = probLossManager.generate_prob();
	if (probabilty > packetLossProb)
	{
		status = _socket.send(ACKpacket, remoteIP, remotePort);
	}
}

void UDPClientManager::SendChallenge(int result)
{
	sf::Packet tryChallenge;
	std::cout << result;

	tryChallenge << _client.id;
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
	matchMakingPacket << _client.id;
	matchMakingPacket << (int)PacketType::MATCHMAKINGMODE;
	matchMakingPacket << result;
	matchMakingPacket << _username;

	Send(matchMakingPacket, sf::IpAddress("127.0.0.1"), 5000);

}

void UDPClientManager::SendDesiredMove()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(80));

		if (commandStack.size() > 0)
		{

			sf::Packet movementPacket;
			movementPacket << _client.id;
			movementPacket << (int)PacketType::MOVEMENT;


			for (int command : commandStack) {
				movementPacket << command;
			}
			SendNonCritical(movementPacket, sf::IpAddress("127.0.0.1"), 5000);
			commandStack.clear();
		}

		//enviar comandos acumulados
	}
}

void UDPClientManager::SendInitialPosition(int x, int y)
{
	sf::Packet initialPosPacket;
	initialPosPacket << _client.id;
	initialPosPacket << (int)PacketType::INITIALPOS;
	initialPosPacket << x;
	initialPosPacket << y;
	Send(initialPosPacket, sf::IpAddress("127.0.0.1"), 5000);
	commandStack.clear();
}
