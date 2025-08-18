#pragma once
#include <map>
#include <enet/enet.h>
#include "Room.h"

#pragma pack(push, 1)
struct PlayerState {
    float posX, posY;
    char ip[32];
    char name[32];
};
#pragma pack(pop)

class RoomManager {
public:
    void SendRoomList(ENetPeer* peer);
    void BroadcastRoomState(int roomId);

    std::map<int, Room> rooms;
    std::map<ENetPeer*, int> peerToRoom;
    int nextRoomId = 1;
};