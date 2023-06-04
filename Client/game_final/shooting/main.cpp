#include "game.h"
#include "UDPClientManager.h"

int main()
{
    //Create and bind the client
    UDPClientManager* client = new UDPClientManager(5001, sf::IpAddress("127.0.0.1"));
    client->Bind();

    //recieve thread
    std::thread rcv(&UDPClientManager::Receive, client);
    rcv.detach();

    //check time stamp thread
    std::thread checkTimeStamp(&UDPClientManager::CheckTimeStamp, client);
    checkTimeStamp.detach();

    //send desired move thread
    std::thread sendMovement(&UDPClientManager::SendDesiredMove, client);
    sendMovement.detach();

    //create graphics and start the thread
    Game g;
    g.run(client); 
}