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
    packet.clear();
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
            int clientId;
            packet >> clientId;

            int type;
            packet >> type;//sempre rebem primer el type

            // Aquí gestionarem tots els diferents casos possibles de missatge (PacketType):
            switch ((PacketType)type)
            {
                case PacketType::TRYCONNECTION:
                {
                    packet >> user;
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
                    std::cout << result << std::endl;

                    sf::Packet challengeStatusPacket;
                    SendACKToClient(remoteIp, remotePort, id);

                    if (result == challengeNumber1 + challengeNumber2)//Afegir a la llista de clients!!
                    {
                        NewConnection tempConnection = _newConnections[std::make_pair(remoteIp, remotePort)];
                        Client tempClient = Client(tempConnection.username, remoteIp, remotePort);
                        tempClient.lastMessageRecievedTs = std::chrono::system_clock::now();
                        tempClient.lastPingSendedTs = std::chrono::system_clock::now();
                        _clients[clientId] = tempClient;

                        _newConnections.erase(std::make_pair(remoteIp, remotePort));
                        challengeStatusPacket << (int)PacketType::CANCONNECT;
                        challengeStatusPacket << clientId;
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

                   
                
                    sf::String username;
                    packet >> username;
                    int id;
                    packet >> id;
                    _clients[clientId].username = username;
                
                    SendACKToClient(remoteIp, remotePort, id);
               

                    if (result == 1)
                    {
                        _matches[currentMatchID] = Match(currentMatchID, clientId, -1);
                        _clients[clientId].matchID = currentMatchID;
                        currentMatchID++;
                        sf::Packet player1Packet;
                        player1Packet << (int)PacketType::PLAYERNUMBER << 1;
                        Send(player1Packet, _clients[clientId].ip, _clients[clientId].port);
                        clientsCreatingMatch.push_back(clientId);
                    }
                    else if (result == 2)
                    {
                        if (clientsCreatingMatch.size() <= 0)
                        {
                            _matches[currentMatchID] = Match(currentMatchID, clientId, -1);
                            _clients[clientId].matchID = currentMatchID;
                            currentMatchID++;
                            sf::Packet player1Packet;
                            player1Packet << (int)PacketType::PLAYERNUMBER << 1;
                            Send(player1Packet, _clients[clientId].ip, _clients[clientId].port);
                            clientsCreatingMatch.push_back(clientId);
                        }
                        else
                        {
                            char _firstUsernameLetter = username[0];

                            int _minimumCharDifference = 255;
                            int _charDifferenceRange;
                            int _matchToJoinId;

                            for (auto m : _matches)
                            {
                                if (m.second.clientID2 == -1)
                                {
                                    char _waitingClientFirstLetter = _clients[m.second.clientID2].username[0];
                                    
                                    _charDifferenceRange = std::abs((int)_firstUsernameLetter - (int)_waitingClientFirstLetter);
                                
                                    if (_charDifferenceRange < _minimumCharDifference)
                                    {
                                        _minimumCharDifference = _charDifferenceRange;

                                        _matchToJoinId = m.first;
                                    }
                                }
                            }

                            _matches[_matchToJoinId].clientID2 = clientId;
                            auto it = std::find(clientsCreatingMatch.begin(), clientsCreatingMatch.end(), _matches[_matchToJoinId].clientID1);
                            if (it != clientsCreatingMatch.end()) {
                                clientsCreatingMatch.erase(it);
                            }
                            

                            _clients[clientId].matchID = _matchToJoinId;
                            sf::Packet player2Packet;
                            player2Packet <<(int) PacketType::PLAYERNUMBER<<2;
                            Send(player2Packet, _clients[clientId].ip, _clients[clientId].port);
                            std::cout << "Player 1 name: " << _clients[_matches[_matchToJoinId].clientID1].username << " ||| " << "Player 2 name: " << _clients[_matches[_matchToJoinId].clientID2].username << std::endl;
                        }
                    }
                
                    break;
                }
                case PacketType::PONG:
                {
                    std::cout << "recieved pong" << std::endl;
                    _clients[clientId].lastPingSendedTs = std::chrono::system_clock::now();
                    _clients[clientId].lastMessageRecievedTs = std::chrono::system_clock::now();
                   // _clients[clientId].pingCounter = 0;
                    _clients[clientId].firstPingSended = false;

                    break;
                }
                case PacketType::ACK:
                {
                    int tempID;
                    packet >> tempID;
                    StorePacketRTT(tempID);
                    std::cout << "recieved ack client" << std::endl;
                    packetsToDelete.push_back(tempID);

                    

                    break;
                }
                case PacketType::MOVEMENT:
                {
                  
                    int TS;
                    int move;
                    int MoveX;
                    int MoveY;

                    
                    

                    while (!packet.endOfPacket())
                    {
                        packet >> TS;
                        packet >> move;
                        packet >> MoveX;
                        packet >> MoveY;

                        movementCmdMap[TS] = MovementCMD{ (UDPServerManager::MoveType)move,MoveX,MoveY };
                      /* */
                    }

                    for (auto it = movementCmdMap.begin(); it != movementCmdMap.end(); ++it) {
                        switch (it->second.moveType)
                        {
                        case MoveType::DOWN:
                        {
                            int tempNewPosX = _clients[clientId].posX + 0;
                            int tempNewPosY = _clients[clientId].posY + 1;

                            sf::Packet newPlayerPosPacket;
                            sf::Packet newRivalPosPacket;

                            if (!(tempNewPosX == it->second.PosX && tempNewPosY == it->second.PosY)) {
                                newPlayerPosPacket << (int)PacketType::MOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                            }
                            else
                            {
                                _clients[clientId].posX = tempNewPosX;
                                _clients[clientId].posY = tempNewPosY;

                                newPlayerPosPacket << (int)PacketType::MOVEMENT << tempNewPosX << tempNewPosY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << _clients[clientId].posX << _clients[clientId].posY;

                            }
                            int rivalID;

                            if (_matches[_clients[clientId].matchID].clientID1 == clientId) {
                                rivalID = _matches[_clients[clientId].matchID].clientID2;
                            }
                            else {
                                rivalID = _matches[_clients[clientId].matchID].clientID1;
                            }
                            SendNonCritical(newPlayerPosPacket, remoteIp, remotePort);
                            SendNonCritical(newRivalPosPacket, _clients[rivalID].ip, _clients[rivalID].port);

                            break;
                        }
                        case MoveType::UP:
                        {

                            int tempNewPosX = _clients[clientId].posX + 0;
                            int tempNewPosY = _clients[clientId].posY - 1;

                          sf::Packet newPlayerPosPacket;
                            sf::Packet newRivalPosPacket;

                            if (!(tempNewPosX == it->second.PosX && tempNewPosY == it->second.PosY)) {
                                newPlayerPosPacket << (int)PacketType::MOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                            }
                            else
                            {
                                _clients[clientId].posX = tempNewPosX;
                                _clients[clientId].posY = tempNewPosY;

                                newPlayerPosPacket << (int)PacketType::MOVEMENT << tempNewPosX << tempNewPosY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << _clients[clientId].posX << _clients[clientId].posY;

                            }
                            int rivalID;
                            
                            if (_matches[_clients[clientId].matchID].clientID1 == clientId) {
                                rivalID = _matches[_clients[clientId].matchID].clientID2;
                            }
                            else {
                                rivalID = _matches[_clients[clientId].matchID].clientID1;
                            }
                            SendNonCritical(newPlayerPosPacket, remoteIp, remotePort);
                            SendNonCritical(newRivalPosPacket, _clients[rivalID].ip, _clients[rivalID].port);



                            break;
                        }
                        case MoveType::RIGHT:
                        {

                            int tempNewPosX = _clients[clientId].posX + 1;
                            int tempNewPosY = _clients[clientId].posY - 0;

                            sf::Packet newPlayerPosPacket;
                            sf::Packet newRivalPosPacket;

                            if (!(tempNewPosX == it->second.PosX && tempNewPosY == it->second.PosY)) {
                                newPlayerPosPacket << (int)PacketType::MOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                            }
                            else
                            {
                                _clients[clientId].posX = tempNewPosX;
                                _clients[clientId].posY = tempNewPosY;

                                newPlayerPosPacket << (int)PacketType::MOVEMENT << tempNewPosX << tempNewPosY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << tempNewPosX << tempNewPosY;

                            }
                            int rivalID;

                            if (_matches[_clients[clientId].matchID].clientID1 == clientId) {
                                rivalID = _matches[_clients[clientId].matchID].clientID2;
                            }
                            else {
                                rivalID = _matches[_clients[clientId].matchID].clientID1;
                            }
                            SendNonCritical(newPlayerPosPacket, remoteIp, remotePort);
                            SendNonCritical(newRivalPosPacket, _clients[rivalID].ip, _clients[rivalID].port);


                            break;
                        }
                        case MoveType::LEFT:
                        {

                            int tempNewPosX = _clients[clientId].posX - 1;
                            int tempNewPosY = _clients[clientId].posY - 0;

                            sf::Packet newPlayerPosPacket;
                            sf::Packet newRivalPosPacket;

                            if (!(tempNewPosX == it->second.PosX && tempNewPosY == it->second.PosY)) {
                                newPlayerPosPacket << (int)PacketType::MOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << _clients[clientId].posX << _clients[clientId].posY;
                            }
                            else
                            {
                                _clients[clientId].posX = tempNewPosX;
                                _clients[clientId].posY = tempNewPosY;

                                newPlayerPosPacket << (int)PacketType::MOVEMENT << tempNewPosX << tempNewPosY;
                                newRivalPosPacket << (int)PacketType::RIVALMOVEMENT << tempNewPosX << tempNewPosY;

                            }
                            int rivalID;
                            
                            if (_matches[_clients[clientId].matchID].clientID1 == clientId) {
                                rivalID = _matches[_clients[clientId].matchID].clientID2;
                            }
                            else {
                                rivalID = _matches[_clients[clientId].matchID].clientID1;
                            }
                            SendNonCritical(newPlayerPosPacket, remoteIp, remotePort);
                            SendNonCritical(newRivalPosPacket, _clients[rivalID].ip, _clients[rivalID].port);

                            break;
                        }

                        }
                    }
                    movementCmdMap.clear();
                    break;
                }
                case PacketType::INITIALPOS:
                {
                    int packetID;
                    packet >> _clients[clientId].posX;
                    packet >> _clients[clientId].posY;
                    packet >> packetID;

                    SendACKToClient(remoteIp,remotePort,packetID);


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
    challengePacket << clientCurrentId++;
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
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (_clients.size() > 0) {

            auto current_time = std::chrono::system_clock::now();

            for (auto client : _clients)
            {

                if (!client.second.firstPingSended)
                {
                    if ((current_time - client.second.lastMessageRecievedTs) >= std::chrono::milliseconds(10000))
                    {
                        if (client.second.pingCounter >= 6) {
                            sf::Packet disconnectionPacket;
                            std::cout << " Disconnected by PING" << std::endl;
                            disconnectionPacket << (int)PacketType::DISCONNECT;
                            SendNonCritical(disconnectionPacket, client.second.ip, client.second.port);
                            _clients.erase(client.second.id);
                        }
                        else
                        {
                            std::cout << " PING " << std::endl;
                            //sendFirstPing
                            client.second.firstPingSended = true;
                            client.second.pingCounter++;
                            std::cout << client.second.pingCounter << std::endl;

                            client.second.lastPingSendedTs = std::chrono::system_clock::now();
                            sf::Packet pingPacket;
                            pingPacket << (int)PacketType::PING;
                            SendNonCritical(pingPacket, client.second.ip, client.second.port);
                        }
                    }
                }
                else {
                    if (((current_time - client.second.lastPingSendedTs) >= std::chrono::milliseconds(2000)) && (client.second.pingCounter < 5))
                    {
                        client.second.pingCounter++;
                        sf::Packet pingPacket;

                        std::cout << " PING" << std::endl;
                        pingPacket << (int)PacketType::PING;
                        SendNonCritical(pingPacket, client.second.ip, client.second.port);
                        client.second.lastPingSendedTs = std::chrono::system_clock::now();
                        //sendFirstPing
                    }
                    else if (client.second.pingCounter >= 5) {
                        sf::Packet disconnectionPacket;
                        std::cout << " Disconnected by PING" << std::endl;
                        disconnectionPacket << (int)PacketType::DISCONNECT;
                        SendNonCritical(disconnectionPacket, client.second.ip, client.second.port);
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
        if (line == "exit") {
            disconnectionPacket << (int)PacketType::DISCONNECT;
            for (std::pair<int, Client> client : _clients) {
                Send(disconnectionPacket, client.second.ip, client.second.port);
            }
            _clients.clear();
            exit(0);
        }
    }
}

UDPServerManager::Status UDPServerManager::SendNonCritical(sf::Packet& packet, sf::IpAddress ip, unsigned short port)
{
    sf::Socket::Status status;
    int probabilty = probLossManager.generate_prob();
    if (probabilty > packetLossProb)
    {
        status = _socket.send(packet, ip, port);
    }

    return Status();
}

void UDPServerManager::StorePacketRTT(int _packetId)
{
    float _rttTime;
    std::chrono::system_clock::time_point _packetFirstTimeSend = packetMap.find(_packetId)->second.firstTimeSent;
    std::chrono::system_clock::time_point _now = std::chrono::system_clock::now();
    
    long mssgRTT = (long)std::chrono::duration_cast<std::chrono::milliseconds>(_now - _packetFirstTimeSend).count();

    _packetRTTs.push_back(mssgRTT);
    std::cout << "mssgRTT" << mssgRTT;
}

void UDPServerManager::CalculateAverageRTT()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        float average = 0;

        if (_packetRTTs.size() > 10) {
            for (int i = _packetRTTs.size() - 11; i < _packetRTTs.size(); i++) {
                average += _packetRTTs[i];
            }

            average /= 10;
        }
        else if (_packetRTTs.size() > 0) {
            for (int i = 0; i < _packetRTTs.size(); i++) {
                average += _packetRTTs[i];
            }

            average /= _packetRTTs.size();
        }

        std::cout << "RTT: " << average << std::endl;
    }
}

void UDPServerManager::ValidateCommands()
{
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));

    }
}






