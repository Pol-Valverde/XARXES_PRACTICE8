#include "game.h"
#include "UDPServerManager.h"
#include <thread>


int main()
{
    srand(time(NULL));
    UDPServerManager* serverManager = new UDPServerManager(5000,sf::IpAddress("127.0.0.1"));
    std::thread rcv(&UDPServerManager::Receive, serverManager);
    rcv.detach();

    std::thread getLine(&UDPServerManager::GetLineFromCin, serverManager);
    getLine.detach();

  //  std::thread checkPing(&UDPServerManager::CheckPing, serverManager);
   // checkPing.detach();

    std::thread checkTimeStamp(&UDPServerManager::CheckTimeStamp, serverManager);
    checkTimeStamp.detach();
    
    std::thread averageRTT(&UDPServerManager::CalculateAverageRTT, serverManager);
    averageRTT.detach();

    Game g;
    g.run(serverManager);
    while (true) {

    }
     
}