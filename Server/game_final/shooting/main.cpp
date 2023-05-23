//#include "game.h"
#include "UDPServerManager.h"
#include <thread>


int main()
{
    UDPServerManager* serverManager = new UDPServerManager(5000,sf::IpAddress("127.0.0.1"));
    std::thread rcv(&UDPServerManager::Receive, serverManager);
    rcv.detach();
    while (true) {

    }
   // Game g;
   // g.run(); 
}