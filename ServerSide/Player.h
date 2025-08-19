#pragma once
#include <string>

struct Player {
    std::string name;
    std::string ip;
    float posX = 100, posY = 100;
    int power = 0;
    bool ghost = false;
};