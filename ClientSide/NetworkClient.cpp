#include "NetworkClient.h"
#include <iostream>
#include <cstring>

NetworkClient::NetworkClient() : client(nullptr), peer(nullptr) {
    if (enet_initialize() != 0) {
        std::cerr << "ENet init failed!\n";
    }
}

NetworkClient::~NetworkClient() {
    Disconnect();
    enet_deinitialize();
}

bool NetworkClient::Connect(const std::string& host, int port) {
    client = enet_host_create(NULL, 1, 2, 0, 0);
    if (!client) return false;

    ENetAddress address;
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    peer = enet_host_connect(client, &address, 2, 0);
    return peer != nullptr;
}

void NetworkClient::Disconnect() {
    if (!peer || !client) return;
    enet_peer_disconnect(peer, 0);

    ENetEvent ev;
    while (enet_host_service(client, &ev, 500) > 0) {
        if (ev.type == ENET_EVENT_TYPE_RECEIVE) enet_packet_destroy(ev.packet);
        if (ev.type == ENET_EVENT_TYPE_DISCONNECT) break;
    }
    enet_host_destroy(client);
    client = nullptr;
    peer = nullptr;
}

void NetworkClient::PollEvents(std::vector<ListedRoom>& rooms, std::vector<PlayerState>& players, int& onlineCount, std::string& errorMsg) {
    ENetEvent ev;
    while (enet_host_service(client, &ev, 0) > 0) {
        if (ev.type == ENET_EVENT_TYPE_RECEIVE) {
            std::string s((char*)ev.packet->data, ev.packet->dataLength);

            if (s.rfind("ONLINE|", 0) == 0) {
                onlineCount = std::atoi(s.substr(7).c_str());
            }
            else if (s.rfind("ERROR", 0) == 0) {
                errorMsg = s;
            }
            else if (s.rfind("ROOMLIST|", 0) == 0) {
                rooms.clear();
                size_t pos = 9;
                while (pos < s.size()) {
                    size_t p1 = s.find('|', pos);
                    if (p1 == std::string::npos) break;
                    std::string idStr = s.substr(pos, p1 - pos);
                    pos = p1 + 1;

                    size_t p2 = s.find('|', pos);
                    if (p2 == std::string::npos) break;
                    std::string rname = s.substr(pos, p2 - pos);
                    pos = p2 + 1;

                    size_t p3 = s.find('|', pos);
                    if (p3 == std::string::npos) break;
                    std::string cntStr = s.substr(pos, p3 - pos);
                    pos = p3 + 1;

                    if (!idStr.empty() && !rname.empty() && !cntStr.empty()) {
                        rooms.push_back({ std::atoi(idStr.c_str()), rname, std::atoi(cntStr.c_str()) });
                    }
                }
            }
            else if (ev.packet->dataLength % sizeof(PlayerState) == 0) {
                size_t count = ev.packet->dataLength / sizeof(PlayerState);
                players.resize(count);
                std::memcpy(players.data(), ev.packet->data, ev.packet->dataLength);
            }

            enet_packet_destroy(ev.packet);
        }
    }
}

void NetworkClient::Send(const std::string& msg, bool reliable) {
    ENetPacket* p = enet_packet_create(msg.c_str(), msg.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    enet_peer_send(peer, 0, p);
    enet_host_flush(client);
}