#include "NetworkManager.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

NetworkManager::NetworkManager() {
    if (enet_initialize() != 0) {
        std::cerr << "ENet init failed!\n";
        exit(-1);
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;

    server = enet_host_create(&address, 32, 2, 0, 0);
    if (!server) {
        std::cerr << "Server create failed!\n";
        exit(-1);
    }

    std::cout << "Server listening on port 1234...\n";
}

NetworkManager::~NetworkManager() {
    if (server) enet_host_destroy(server);
    enet_deinitialize();
}

void NetworkManager::Run() {
    ENetEvent ev;
    while (enet_host_service(server, &ev, 1000) >= 0) {
        switch (ev.type) {
        case ENET_EVENT_TYPE_CONNECT:   HandleConnect(ev); break;
        case ENET_EVENT_TYPE_RECEIVE:   HandleReceive(ev); break;
        case ENET_EVENT_TYPE_DISCONNECT: HandleDisconnect(ev); break;
        default: break;
        }
    }
}

void NetworkManager::HandleConnect(ENetEvent& ev) {
    std::cout << "Client connected.\n";
    connectedClients++;

    std::string msg = "ONLINE|" + std::to_string(connectedClients);
    ENetPacket* p = enet_packet_create(msg.c_str(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, p);
    enet_host_flush(server);
}

void NetworkManager::HandleReceive(ENetEvent& ev) {
    std::string msg((char*)ev.packet->data, ev.packet->dataLength);
    enet_packet_destroy(ev.packet);

    if (msg == "GETROOMS") {
        roomManager.SendRoomList(ev.peer);
    }
    else if (msg.rfind("CREATE ", 0) == 0) {
        std::string playerName = msg.substr(7);
        std::string ipStr = GetIPFromPeer(ev.peer);
        std::string roomName = playerName + "'s Room";

        bool exists = false;
        for (auto& kv : roomManager.rooms) {
            if (kv.second.name == roomName) {
                exists = true; break;
            }
        }

        if (exists) {
            std::string err = "ERROR Room with this name already exists!";
            ENetPacket* p = enet_packet_create(err.c_str(), err.size(), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(ev.peer, 0, p);
            enet_host_flush(server);
        }
        else {
            Room room;
            room.name = roomName;

            Player p;
            p.name = playerName;
            p.ip = ipStr;

            room.players[ev.peer] = p;
            roomManager.rooms[roomManager.nextRoomId] = room;
            roomManager.peerToRoom[ev.peer] = roomManager.nextRoomId;

            std::cout << "Room created: " << room.name << " (id " << roomManager.nextRoomId << ")\n";
            roomManager.BroadcastRoomState(roomManager.peerToRoom[ev.peer]);
            roomManager.nextRoomId++;
        }
    }
    else if (msg.rfind("JOIN ", 0) == 0) {
        std::istringstream iss(msg.substr(5));
        int roomId;
        std::string playerName;
        iss >> roomId >> playerName;

        if (roomManager.rooms.find(roomId) == roomManager.rooms.end()) {
            std::string err = "ERROR Room not found!";
            ENetPacket* p = enet_packet_create(err.c_str(), err.size(), ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(ev.peer, 0, p);
            enet_host_flush(server);
            return;
        }

        Room& r = roomManager.rooms[roomId];

        // isim kontrolü
        for (auto& kv : r.players) {
            if (kv.second.name == playerName) {
                std::string err = "ERROR Name already taken in this room!";
                ENetPacket* p = enet_packet_create(err.c_str(), err.size(), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(ev.peer, 0, p);
                enet_host_flush(server);
                return;
            }
        }

        Player p;
        p.name = playerName;
        p.ip = GetIPFromPeer(ev.peer);

        r.players[ev.peer] = p;
        roomManager.peerToRoom[ev.peer] = roomId;

        std::cout << "Player " << playerName << " joined room " << r.name << "\n";
        roomManager.BroadcastRoomState(roomId);
    }
    else if (msg == "W" || msg == "A" || msg == "S" || msg == "D") {
        if (roomManager.peerToRoom.find(ev.peer) != roomManager.peerToRoom.end()) {
            int roomId = roomManager.peerToRoom[ev.peer];
            auto& player = roomManager.rooms[roomId].players[ev.peer];

            if (msg == "W") player.posY -= 4;
            if (msg == "S") player.posY += 4;
            if (msg == "A") player.posX -= 4;
            if (msg == "D") player.posX += 4;

            roomManager.BroadcastRoomState(roomId);
        }
    }
}

void NetworkManager::HandleDisconnect(ENetEvent& ev) {
    std::cout << "Client disconnected.\n";
    connectedClients--;

    std::string msg = "ONLINE|" + std::to_string(connectedClients);
    ENetPacket* p = enet_packet_create(msg.c_str(), msg.size(), ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, p);
    enet_host_flush(server);

    if (roomManager.peerToRoom.find(ev.peer) != roomManager.peerToRoom.end()) {
        int roomId = roomManager.peerToRoom[ev.peer];
        if (roomManager.rooms.find(roomId) != roomManager.rooms.end()) {
            Room& room = roomManager.rooms[roomId];

            if (!room.players.empty() && room.players.begin()->first == ev.peer) {
                for (auto& kv : room.players) {
                    if (kv.first != ev.peer) {
                        std::string err = "ERROR Room owner has left the game!";
                        ENetPacket* p = enet_packet_create(err.c_str(), err.size(), ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(kv.first, 0, p);
                        enet_host_flush(server);
                        roomManager.peerToRoom.erase(kv.first);
                    }
                }
                roomManager.rooms.erase(roomId);
            }
            else {
                room.players.erase(ev.peer);
                if (room.players.empty()) roomManager.rooms.erase(roomId);
                else roomManager.BroadcastRoomState(roomId);
            }
        }
        roomManager.peerToRoom.erase(ev.peer);
    }
}