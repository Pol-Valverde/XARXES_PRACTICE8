#pragma once

#include <SFML\Network.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include "PacketLoss.h"

class UDPServerManager
{

public:
    enum class MoveType
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        SHOOTLEFT,
        SHOOTLEFTUP,
        SHOOTRIGHT,
        SHOOTRIGHTUP,
        SHOOTUP,
        SHOOTLEFTDOWN,
        SHOOTDOWN,
        SHOOTRIGHTDOWN
    };

    struct Client // NEW: changed Client structure
    {
        // NEW: removed "socket"
        sf::IpAddress ip; // NEW: added this
        unsigned short port; // NEW: added this
        std::string username;
        unsigned int id; // NEW: added this
        unsigned int matchID; // NEW: added this
        std::chrono::system_clock::time_point ts; // NEW: added this
        std::chrono::system_clock::time_point lastMessageRecievedTs;
        std::chrono::system_clock::time_point lastPingSendedTs;
        bool firstPingSended = false;
        int pingCounter = 0;
        int posX;
        int posY;

        Client() = default;

        Client(std::string _username, sf::IpAddress _ip, unsigned short _port)
            : username(_username), ip(_ip), port(_port) {}
    };
    struct MovementCMD 
    {
        MoveType moveType;
        int PosX;
        int PosY;
    };

    
    struct NewConnection
    {
        sf::IpAddress ip;
        unsigned short port;
        std::string username;
        int ch_ts;
        int challenge1;
        int challenge2;
        int solution;

        NewConnection() = default;

        NewConnection(sf::IpAddress _ip, unsigned short _port, std::string _username, int _challenge1, int _challenge2, int _solution)
            : ip(_ip), port(_port), username(_username), challenge1(_challenge1), challenge2(_challenge2), solution(_solution) {}
    };

    struct Match
    {
        int matchId;
        int clientID1;
        int clientID2;
        Match() = default;
        Match(int id, int c1, int c2) : matchId(id), clientID1(c1), clientID2(c2) {}

    };

    struct PacketInfo // Serveix tant per paquets enviats com pels paquets de tipus ACK.
    {
        int id;
        sf::Packet packet;
        std::chrono::system_clock::time_point firstTimeSent; // ADDED THIS! // First TimeStamp
        std::chrono::system_clock::time_point timeSent; // Latest TimeStamp
        sf::IpAddress remoteIp;
        unsigned short remotePort;

        PacketInfo() = default;

        PacketInfo(int _id, sf::Packet _packet, std::chrono::system_clock::time_point _firstTimeSent, std::chrono::system_clock::time_point _timeSent, sf::IpAddress _remoteIp, unsigned short _remotePort)
            : id(_id), packet(_packet), firstTimeSent(_firstTimeSent), timeSent(_timeSent), remoteIp(_remoteIp), remotePort(_remotePort) {}

        PacketInfo(int _id, sf::Packet _packet, std::chrono::system_clock::time_point _timeSent, sf::IpAddress _remoteIp, unsigned short _remotePort)
            : id(_id), packet(_packet), timeSent(_timeSent), remoteIp(_remoteIp), remotePort(_remotePort) {}
    };

    std::map<int, Client> _clients;
    std::map<int, Match> _matches;
    int currentMatchID = 0;
    std::vector<int> clientsCreatingMatch;
    int packetLossProb = 5;
    std::vector<float> _packetRTTs;

    enum class Status
    {
        Done,               // The socket has sent / received the data correctly
        Error,              // An unexpected error happened
        Connected,          // The socket is connected and ready to work
        Disconnected        // The TCP socket is disconnected
    };
private:

    enum class PacketType
    {
        TRYCONNECTION,      // Packet to start a connection
        CANCONNECT,         // Packet to confirm connection
        CANNOTCONNECT,      // Packet to confirm failed connection
        CHALLENGE,          // Packet to send challenge question and challenge answer
        CHALLENGEFAILED,    // Captcha failed
        RETRYCHALLENGE,     // Retry challenge
        MATCHMAKINGMODE,
        PLAYERNUMBER,
        MOVEMENT,
        RIVALMOVEMENT,
        INITIALPOS,
        PING,
        PONG,
        MESSAGE,            // Packet to send a message to the global chat
        ACK,
        DISCONNECT          // Packet to disconnect
    };
  
private:
    // ------ VARIABLES: ------
    sf::UdpSocket _socket;
    unsigned short _port;
    sf::IpAddress _ip;
    int _nextClientId; //se li va sumant un cada cop que es conecta un client
    PacketLoss probLossManager;
    std::map<std::pair<sf::IpAddress, unsigned short>, NewConnection> _newConnections;//clients que s'estan intentant conectar
    int packetCount; //id del paquet
    std::map<int, PacketInfo> packetMap;
    std::map<int, MovementCMD> movementCmdMap;
    std::vector<int> packetsToDelete;
    int challengeNumber1;
    int challengeNumber2;
    int clientCurrentId = 0;
public:
    // ------ CONSTRUCTOR: ------
    UDPServerManager(unsigned short port, sf::IpAddress ip) 
    {
        _socket.setBlocking(true); //!!Important per la linea d'execucio en el receive
        _socket.bind(port);
    };

    // ------ METHODS: ------
    Status Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port);
    Status ReSend(sf::Packet& packet, int packetId, sf::IpAddress ip, unsigned short port);
    void Receive();
    void CreateChallenge(const sf::IpAddress& remoteIp, unsigned short remotePort,PacketType pt);
    //Status Connect();
    Status Listen();
    void Disconnect();
    void SendACKToClient(sf::IpAddress remoteIP, unsigned short remotePort, int id);
    void CheckPing();
    void CheckTimeStamp();
    void GetLineFromCin();
    Status SendNonCritical(sf::Packet& packet, sf::IpAddress ip, unsigned short port);
    void StorePacketRTT(int _packetId);
    void CalculateAverageRTT();
    void ValidateCommands();
};

