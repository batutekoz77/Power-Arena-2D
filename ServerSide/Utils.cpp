#include "Utils.h"

std::string GetIPFromPeer(ENetPeer* peer) {
    char ipStr[64];
    enet_address_get_host_ip(&peer->address, ipStr, sizeof(ipStr));
    return std::string(ipStr);
}