#pragma once

#include <SFML\Network.hpp>
#include <iostream>
#include <chrono>
#include "PacketLoss.h"

class UDPClientManager
{

    struct Client // NEW: changed Client structure
    {
        // NEW: removed "socket"
        sf::IpAddress ip; // NEW: added this
        unsigned short port; // NEW: added this
        std::string username;
        unsigned int id; // NEW: added this
        int ts; // NEW: added this

        Client(std::string _username) {
            username = _username;
        }
        Client() = default;
        Client(std::string _username, int _id)
            : username(_username), id(_id) {}
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
        PING,
        PONG,
        MESSAGE,            // Packet to send a message to the global chat
        ACK,
        DISCONNECT          // Packet to disconnect
    };

    //VARIBLES:
    sf::UdpSocket _socket;
    unsigned short _port = 5001;
    sf::IpAddress _ip;
    PacketLoss probLossManager;
    std::map<int, PacketInfo> packetMap;
    std::vector<int> packetsToDelete;
    int packetCount = 0;
    int id;
    bool _setId = true;
    int clientId;
    
public:
    int challengeNumber1, challengeNumber2;
    bool isChallenge = false;
    bool selectMatchMakingOption = false;
    bool matchMaking = false;
    bool _startPlaying;
    Client _client;
    int packetLossProb = 5;
    sf::String _username;
    UDPClientManager() = default;
    UDPClientManager(unsigned short port, sf::IpAddress ip);

    Status Send(sf::Packet& packet, sf::IpAddress ip, unsigned short port);
    Status SendNonCritical(sf::Packet& packet, sf::IpAddress ip, unsigned short port);
    Status ReSend(sf::Packet& packet, int packetId, sf::IpAddress ip, unsigned short port);
    void Bind();
    void TryConnection(sf::String user);
    void Receive();
    void Disconnect();
    void CheckTimeStamp();
    void SendACKToServer(sf::IpAddress remoteIP, unsigned short remotePort, int id);
    void SendChallenge(int result);
    void SendSelectMatchMakingType(int result);
};

