#include "RoomManager.h"
#include <vector>
#include <cstring>
#include <iostream>

void RoomManager::SendRoomList(ENetPeer* peer) {
    std::string msg = "ROOMLIST|";
    for (auto& [id, room] : rooms) {
        msg += std::to_string(id) + "|" + room.name + "|" + std::to_string(room.players.size()) + "|";
    }

    ENetPacket* p = enet_packet_create(msg.c_str(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, p);
}

void RoomManager::BroadcastRoomState(int roomId) {
    if (rooms.find(roomId) == rooms.end()) return;
    auto& room = rooms[roomId];

    std::vector<PlayerState> buf;
    for (auto& [peer, pl] : room.players) {
        PlayerState ps;
        ps.posX = pl.posX;
        ps.posY = pl.posY;
        strncpy_s(ps.name, pl.name.c_str(), sizeof(ps.name));
        strncpy_s(ps.ip, pl.ip.c_str(), sizeof(ps.ip));
        buf.push_back(ps);
    }

    size_t size = buf.size() * sizeof(PlayerState);
    ENetPacket* p = enet_packet_create(buf.data(), size, ENET_PACKET_FLAG_RELIABLE);

    for (auto& [peer, _] : room.players) {
        enet_peer_send(peer, 0, p);
    }
}