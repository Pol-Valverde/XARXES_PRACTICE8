#include "game.h"
#include "UDPClientManager.h"

int main()
{
    UDPClientManager* client = new UDPClientManager(5001, sf::IpAddress("127.0.0.1"));
    client->Bind(); 
    std::thread rcv(&UDPClientManager::Receive, client);
    rcv.detach();
    Game g;
    g.run(client); 
}