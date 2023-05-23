#include "UDPServerManager.h"

#define PKT_LOSS_PROB 5// 5 sobre 1000 -> 0.5%
#define PACKET_TIMEOUT_IN_MILLIS 1000 // milliseconds

UDPServerManager::Status UDPServerManager::Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
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
            packet >> user;
            // Aquí gestionarem tots els diferents casos possibles de missatge (PacketType):
            switch ((PacketType)type)
            {
            case PacketType::TRYCONNECTION:
            {
                std::cout << "--Username-- "<<std::endl << user.toAnsiString()<< std::endl;
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
