#include "UDPServerManager.h"

#define PKT_LOSS_PROB 5// 5 sobre 1000 -> 0.5%
#define PACKET_TIMEOUT_IN_MILLIS 1000 // milliseconds

UDPServerManager::Status UDPServerManager::Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
    sf::Socket::Status status;
    PacketInfo packetInfo = PacketInfo{ packetCount, packet, std::chrono::system_clock::now(), std::chrono::system_clock::now(), ip, port };
    packet << packetCount;
    packetMap[packetCount++] = packetInfo;
    status = _socket.send(packet, ip, port);
    return Status();
}

void UDPServerManager::Receive()
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
            std::cout << "status is done" << std::endl;
            int type;
            packet >> type;//sempre rebem primer el type
            
            // Aqu� gestionarem tots els diferents casos possibles de missatge (PacketType):
            switch ((PacketType)type)
            {
                case PacketType::TRYCONNECTION:
                {
                    packet >> user;
                    std::cout << "--Username-- "<<std::endl << user.toAnsiString()<< std::endl;
                    CreateChallenge(remoteIp, remotePort,PacketType::CHALLENGE);

                    NewConnection newConnection(remoteIp, remotePort, user.toAnsiString(), challengeNumber1, challengeNumber2, challengeNumber1 + challengeNumber2);
                    _newConnections[std::make_pair(remoteIp, remotePort)] = newConnection;

                    

                    break;
                }
                case PacketType::CHALLENGE:
                {
                    int result;
                    packet >> result;
                    std::cout << result<<std::endl;

                    sf::Packet challengeStatusPacket;

                    if (result == challengeNumber1 + challengeNumber2)//Afegir a la llista de clients!!
                    {
                        std::cout << "TRUEEEE";

                        clientCurrentId++;
                        NewConnection tempConnection = _newConnections[std::make_pair(remoteIp, remotePort)];
                        Client tempClient = Client(tempConnection.username, tempConnection.ip, tempConnection.port);
                        _clients[clientCurrentId] = tempClient;
                        _newConnections.erase(std::make_pair(remoteIp, remotePort));
                        challengeStatusPacket << (int)PacketType::CANCONNECT;
                        challengeStatusPacket << clientCurrentId;
                        Send(challengeStatusPacket, remoteIp, remotePort);
                    }
                    else
                    {
                        std::cout << "False";
                        CreateChallenge(remoteIp, remotePort, PacketType::CANNOTCONNECT);
                    }

                    break;
                }
                case PacketType::MATCHMAKINGMODE:
                {
                    int result;
                    packet >> result;
                    std::cout << "\n\n Result OF MATCHMAKINGTYPE --->"<<result<<std::endl;

                    if (result == 1)
                    {
                        clientsCreatingMatch.push_back(clientCurrentId);
                    }
                    else if (result == 2 )
                    {
                        if (clientsCreatingMatch.size() <= 0) {
                            clientsCreatingMatch.push_back(clientCurrentId);
                        }
                        else {
                            _matches[currentMatchID] = Match(currentMatchID, clientsCreatingMatch[0], currentMatchID);
                            clientsCreatingMatch.erase(clientsCreatingMatch.begin());
                            //TODO: connect both players
                        }
                        
                    }

                }
            }
        }
    }
}

void UDPServerManager::CreateChallenge(const sf::IpAddress& remoteIp, unsigned short remotePort,PacketType pt)
{
    sf::Packet challengePacket;
    challengePacket << (int)pt;
    challengeNumber1 = rand() % 51;
    challengeNumber2 = rand() % 51;
    challengePacket << challengeNumber1;
    challengePacket << challengeNumber2;
    Send(challengePacket, remoteIp, remotePort);
}

UDPServerManager::Status UDPServerManager::Listen()
{
    _socket.bind(5000);
    return Status();
}




//UDPServerManager::Status UDPServerManager::Connect()
//{
//    sf::Packet packet;
//    packet << (int)PacketType::TRYCONNECTION;//primer posem el type
//    packet << packetCount++;//despres la id
//    UDPServerManager::Status status;
//
//    int probValue = probLossManager.generate_prob();
//    //calculate probability
//    //if (probValue > PKT_LOSS_PROB)
//        status = Send(packet, _ip, _port, new std::string());
//    //else {
//       // std::cout << "error when sending packet" << std::endl;
//   // }
//
//    if (status != UDPServerManager::Status::Done)
//    {
//        std::cout << "\t> A packet has been lost.";
//        return Status::Error;
//    }
//
//    return status;
//}
