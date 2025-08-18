#pragma once
#include "raylib.h"
#include <string>

bool Button(Rectangle bounds, const char* text);
void DrawTextCentered(const std::string& text, int x, int y, int fontSize, Color color);