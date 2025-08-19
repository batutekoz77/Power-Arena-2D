#pragma once
#include <string>
#include <vector>

#pragma pack(push, 1)
struct PlayerState {
    float posX, posY;
    char ip[32];
    char name[32];
    int power;
    bool ghost;
};
#pragma pack(pop)

struct ListedRoom {
    int id;
    std::string name;
    int count;
};

enum class ScreenState {
    MAIN_MENU,
    JOIN_MENU,
    PLAYING
};