#pragma once
#include <enet/enet.h>
#include <string>
#include <vector>
#include "GameState.h"

class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();

    bool Connect(const std::string& host, int port);
    void Disconnect();
    void PollEvents(std::vector<ListedRoom>& rooms, std::vector<PlayerState>& players, int& onlineCount, std::string& errorMsg);
    void Send(const std::string& msg, bool reliable = true);
    ENetPeer* GetPeer() { return peer; }
    ENetHost* GetHost() { return client; }

private:
    ENetHost* client;
    ENetPeer* peer;
};