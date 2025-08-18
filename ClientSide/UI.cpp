#include "UI.h"

bool Button(Rectangle bounds, const char* text) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, bounds);
    DrawRectangleRec(bounds, hover ? DARKGRAY : LIGHTGRAY);
    DrawRectangleLines((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height, BLACK);
    int tw = MeasureText(text, 20);
    DrawText(text, (int)(bounds.x + (bounds.width - tw) / 2), (int)(bounds.y + (bounds.height - 20) / 2), 20, BLACK);
    return (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

void DrawTextCentered(const std::string& text, int x, int y, int fontSize, Color color) {
    int w = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), x - w / 2, y, fontSize, color);
}