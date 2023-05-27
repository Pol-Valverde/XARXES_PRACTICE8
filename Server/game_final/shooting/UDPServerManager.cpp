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
            int type;
            packet >> type;//sempre rebem primer el type
            
            // Aquí gestionarem tots els diferents casos possibles de missatge (PacketType):
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

                        clientCurrentId++;
                        NewConnection tempConnection = _newConnections[std::make_pair(remoteIp, remotePort)];
                        Client tempClient = Client(tempConnection.username, tempConnection.ip, tempConnection.port);
                        tempClient.lastMessageRecievedTs = std::chrono::system_clock::now();
                        tempClient.lastPingSendedTs = std::chrono::system_clock::now();
                        _clients[clientCurrentId] = tempClient;

                        _newConnections.erase(std::make_pair(remoteIp, remotePort));
                        challengeStatusPacket << (int)PacketType::CANCONNECT;
                        challengeStatusPacket << clientCurrentId;
                        Send(challengeStatusPacket, remoteIp, remotePort);
                    }
                    else
                    {
                        CreateChallenge(remoteIp, remotePort, PacketType::CANNOTCONNECT);
                        
                    }

                    break;
                }
                case PacketType::MATCHMAKINGMODE:
                {
                    int result;
                    packet >> result;

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
                    break;
                }
                case PacketType::PONG:
                {
                    int tempId;
                    packet >> tempId;
                    std::cout << "recieved pong" << std::endl;
                    _clients[tempId].firstPingSended = false;
                    _clients[tempId].lastPingSendedTs = std::chrono::system_clock::now();
                    _clients[tempId].lastMessageRecievedTs = std::chrono::system_clock::now();
                    _clients[tempId].pingCounter = 0;

                    break;
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

void UDPServerManager::CheckPing()
{
    while (true) 
    {
        if (_clients.size() > 0) {

            auto current_time = std::chrono::system_clock::now();
            for (auto client : _clients)
            {
                if (!(client.second.firstPingSended))
                {
                   
                    if ((current_time - client.second.lastMessageRecievedTs) >= std::chrono::milliseconds(10000))
                    {
                        //sendFirstPing
                        client.second.firstPingSended = true;
                        client.second.lastPingSendedTs = std::chrono::system_clock::now();
                        sf::Packet pingPacket;
                        std::cout << "FIRST PING" << std::endl;
                        pingPacket << (int)PacketType::PING;
                        Send(pingPacket, client.second.ip, client.second.port);

                        std::cout << _clients.size();
                        
                    }
                }
                else {
                    if ( ((current_time - client.second.lastPingSendedTs) >= std::chrono::milliseconds(2000))  && (client.second.pingCounter<5) )
                    {
                        client.second.pingCounter++;
                        sf::Packet pingPacket;
                        
                        std::cout << " PING" << std::endl;
                        pingPacket << (int)PacketType::PING;
                        Send(pingPacket, client.second.ip, client.second.port);
                        client.second.lastPingSendedTs = std::chrono::system_clock::now();
                        //sendFirstPing
                    }
                    else if (client.second.pingCounter > 5) {
                        sf::Packet disconnectionPacket;
                        std::cout << " Disconnected by PING" << std::endl;
                        disconnectionPacket << (int)PacketType::DISCONNECT;
                        Send(disconnectionPacket, client.second.ip, client.second.port);
                        _clients.erase(client.second.id);
                    }
                }
            }
        }
    }

}

void UDPServerManager::GetLineFromCin()
{
    std::string line;

    while (true)
    {
        sf::Packet disconnectionPacket;
        std::getline(std::cin, line);
        if ( line == "exit") {
            disconnectionPacket << (int)PacketType::DISCONNECT;
            for (std::pair<int, Client> client : _clients) {
                Send(disconnectionPacket, client.second.ip, client.second.port);
            }
            _clients.clear();
            exit(0);
        }
    }
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
