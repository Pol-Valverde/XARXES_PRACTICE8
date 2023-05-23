#include "PacketLoss.h"

int PacketLoss::generate_prob()
{
    return (rand() % 1000) + 1;
}
