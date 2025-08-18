#pragma once
#include <enet/enet.h>
#include "RoomManager.h"

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void Run();

private:
    ENetHost* server = nullptr;
    RoomManager roomManager;
    int connectedClients = 0;

    void HandleConnect(ENetEvent& ev);
    void HandleReceive(ENetEvent& ev);
    void HandleDisconnect(ENetEvent& ev);
};