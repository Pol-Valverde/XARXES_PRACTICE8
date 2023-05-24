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
            
            // Aquí gestionarem tots els diferents casos possibles de missatge (PacketType):
            switch ((PacketType)type)
            {
                case PacketType::TRYCONNECTION:
                {
                    packet >> user;
                    std::cout << "--Username-- "<<std::endl << user.toAnsiString()<< std::endl;

                    NewConnection newConnection(remoteIp, remotePort, user.toAnsiString(), challengeNumber1, challengeNumber1, challengeNumber1 + challengeNumber1);
                    _newConnections[std::make_pair(remoteIp, remotePort)] = newConnection;

                    sf::Packet challengePacket;
                    challengePacket << (int)PacketType::CHALLENGE;
                    challengePacket << challengeNumber1;
                    challengePacket << challengeNumber1;
                    Send(challengePacket,remoteIp,remotePort);
                    break;
                }
                case PacketType::CHALLENGE:
                {
                    int result;
                    packet >> result;
                    std::cout << result<<std::endl;
                    if (result == challengeNumber1 + challengeNumber1)
                    {
                        std::cout << "TRUEEEE";
                    }
                    else
                        std::cout << "False";

                    break;
                }
            }
        }
    }
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
