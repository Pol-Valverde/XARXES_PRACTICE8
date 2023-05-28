#include "UDPServerManager.h"


#define PACKET_TIMEOUT_IN_MILLIS 500 // milliseconds

UDPServerManager::Status UDPServerManager::Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
    int probabilty = probLossManager.generate_prob();
    std::cout << "probability: " << probabilty << std::endl;

    sf::Socket::Status status;
    PacketInfo packetInfo = PacketInfo{ packetCount, packet, std::chrono::system_clock::now(), std::chrono::system_clock::now(), ip, port };
    packet << packetCount;
    packetMap[packetCount++] = packetInfo; // packets to ReSend
    if (probabilty > packetLossProb)
    {
        status = _socket.send(packet, ip, port);
    }
    return Status();
}

UDPServerManager::Status UDPServerManager::ReSend(sf::Packet& packet, int packetId, sf::IpAddress ip, unsigned short port)
{
    std::cout << "RESEND" << std::endl;

    // Fill packet with data:
    sf::Socket::Status status;
    PacketInfo packetInfo = PacketInfo{ packetId, packet, std::chrono::system_clock::now(), ip, port };
    if (packetMap.find(packetId) != packetMap.end())
    packetMap[packetId] = packetInfo;

    int probabilty = probLossManager.generate_prob();
    if (probabilty > packetLossProb)
    {
        status = _socket.send(packet, ip, port);
    }
    else
        status = sf::Socket::Status::Error;
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
                    CreateChallenge(remoteIp, remotePort, PacketType::CHALLENGE);

                    NewConnection newConnection(remoteIp, remotePort, user.toAnsiString(), challengeNumber1, challengeNumber2, challengeNumber1 + challengeNumber2);
                    _newConnections[std::make_pair(remoteIp, remotePort)] = newConnection;

                    break;
                }
                case PacketType::CHALLENGE:
                {
                    int result, id;
                    packet >> result;
                    packet >> id;
                    std::cout << "Challenge packet id: " << id;
                    std::cout << result<<std::endl;

                    sf::Packet challengeStatusPacket;
                    SendACKToClient(remoteIp, remotePort, id);

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
                    int id;
                    packet >> id;

                    SendACKToClient(remoteIp, remotePort, id);
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
                    _clients[tempId].lastPingSendedTs = std::chrono::system_clock::now();
                    _clients[tempId].lastMessageRecievedTs = std::chrono::system_clock::now();
                    _clients[tempId].pingCounter = 0;
                    _clients[tempId].firstPingSended = false;

                    break;
                }
                case PacketType::ACK:
                {
                    int tempID;
                    packet >> tempID;

                    std::cout << "recieved ack client" << std::endl;
                    packetsToDelete.push_back(tempID);

                    break;
                }
            }
        }
    }
}

void UDPServerManager::CreateChallenge(const sf::IpAddress& remoteIp, unsigned short remotePort, PacketType pt)
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

void UDPServerManager::SendACKToClient(sf::IpAddress remoteIP, unsigned short remotePort, int id)
{
    sf::Packet ACKpacket;
    sf::Socket::Status status;

    ACKpacket << (int)PacketType::ACK;
    ACKpacket << id;
    int probabilty = probLossManager.generate_prob();
    if (probabilty > packetLossProb)
    {
        status = _socket.send(ACKpacket, remoteIP, remotePort);
    }
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

void UDPServerManager::CheckTimeStamp()
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

                    ReSend(packet.second.packet, packet.first, packet.second.remoteIp, packet.second.remotePort); // ADDED THIS!
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

            std::cout << "PacketMap size: " << packetsToDelete.size() << std::endl;
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

void UDPServerManager::SendNonCritical(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
    sf::Socket::Status status;
    int probabilty = probLossManager.generate_prob();
    if (probabilty > packetLossProb)
    {
        status = _socket.send(packet, ip, port);
    }
  

}






