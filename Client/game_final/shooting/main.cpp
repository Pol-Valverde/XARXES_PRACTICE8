#include "game.h"
#include "UDPClientManager.h"
int main()
{
    UDPClientManager* client = new UDPClientManager(5001, sf::IpAddress("127.0.0.1"));
    client->Bind(); 
    Game g;
    g.run(client); 
}