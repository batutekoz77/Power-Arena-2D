#pragma once
#include <map>
#include <string>
#include <vector>
#include <enet/enet.h>
#include "Player.h"

struct Room {
    std::string name;
    std::map<ENetPeer*, Player> players;
};