#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include "raylib.h"
#include "NetworkClient.h"
#include "UI.h"
#include "Utils.h"
#include "GameState.h"
#include <iostream>

int main() {
    NetworkClient net;
    if (!net.Connect("127.0.0.1", 1234)) {
        std::cerr << "Connection failed!\n";
        return -1;
    }

    InitWindow(900, 650, "Square Fighters - V1.0");
    SetTargetFPS(60);

    ScreenState state = ScreenState::MAIN_MENU;
    std::string nameInput, myName, errorMessage;
    bool showNameError = false, typing = true;
    int onlineCount = 0;

    std::vector<ListedRoom> rooms;
    std::vector<PlayerState> players;

    while (!WindowShouldClose()) {
        net.PollEvents(rooms, players, onlineCount, errorMessage);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (state == ScreenState::MAIN_MENU) {
            DrawText("Enter Name:", 300, 200, 22, BLACK);

            // Input box
            Rectangle nameBox = { 300, 235, 300, 40 };
            DrawRectangleRec(nameBox, LIGHTGRAY);
            DrawRectangleLines((int)nameBox.x, (int)nameBox.y, (int)nameBox.width, (int)nameBox.height, BLACK);
            DrawText(nameInput.c_str(), (int)nameBox.x + 10, (int)nameBox.y + 10, 20, BLACK);

            if (typing) {
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= 32 && key <= 125 && nameInput.size() < 12) {
                        nameInput.push_back((char)key);
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && !nameInput.empty()) {
                    nameInput.pop_back();
                }
            }

            bool nameOk = IsValidName(nameInput);
            if (!nameOk) DrawText("Name must be 3-12 characters.", 300, 285, 19, RED);

            if (Button({ 300, 330, 300, 50 }, "Create Room")) {
                if (nameOk) {
                    myName = nameInput;
                    net.Send("CREATE " + myName);
                    state = ScreenState::PLAYING;
                }
                else showNameError = true;
            }

            if (Button({ 300, 390, 300, 50 }, "Join Room")) {
                if (nameOk) {
                    myName = nameInput;
                    net.Send("GETROOMS");
                    state = ScreenState::JOIN_MENU;
                }
                else showNameError = true;
            }

            if (showNameError && nameOk) showNameError = false;
            if (showNameError) DrawText("Please enter a valid name first.", 300, 455, 18, RED);
            if (!errorMessage.empty()) DrawText(errorMessage.c_str(), 300, 490, 19, RED);

            DrawText(("Online: " + std::to_string(onlineCount)).c_str(), 10, GetScreenHeight() - 25, 18, DARKGRAY);
            DrawText("Coded by @batutekoz", GetScreenWidth() - 220, GetScreenHeight() - 25, 18, DARKGRAY);
        }
        else if (state == ScreenState::JOIN_MENU) {
            DrawText("Active Rooms:", 60, 40, 24, BLACK);
            float y = 90.0f;
            for (auto& lr : rooms) {
                std::string label = lr.name + " (" + std::to_string(lr.count) + ")";
                if (Button({ 60, y, 600, 40 }, label.c_str())) {
                    net.Send("JOIN " + std::to_string(lr.id) + " " + myName);
                    state = ScreenState::PLAYING;
                }
                y += 50.0f;
            }

            if (Button({ 680, 40, 160, 40 }, "Refresh")) net.Send("GETROOMS");
            if (Button({ 680, 90, 160, 40 }, "Back")) state = ScreenState::MAIN_MENU;
        }
        else if (state == ScreenState::PLAYING) {
            PlayerState* me = nullptr;
            for (auto& pl : players) {
                if (std::strcmp(pl.name, myName.c_str()) == 0) {
                    me = &pl;
                    break;
                }
            }

            if (me && me->power == 0) {
                // Arka plan paneli
                DrawRectangle(250, 80, 400, 300, Fade(RAYWHITE, 0.95f));
                DrawRectangleLines(250, 80, 400, 300, DARKBROWN);

                DrawText("Select Your Super Power", 270, 100, 24, DARKBROWN);
                DrawText("Choose wisely, this will define your playstyle!", 270, 130, 16, GRAY);

                Vector2 mousePos = GetMousePosition();

                auto drawButton = [&](Rectangle rect, const char* label, const char* desc, int id, Color highlight) {
                    bool hover = CheckCollisionPointRec(mousePos, rect);
                    Color bg = hover ? highlight : LIGHTGRAY;

                    DrawRectangleRec(rect, bg);
                    DrawRectangleLinesEx(rect, 2, DARKBROWN);

                    DrawText(label, rect.x + 10, rect.y + 8, 20, BLACK);
                    DrawText(desc, rect.x + 10, rect.y + 30, 14, DARKGRAY);

                    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        std::string msg = "POWER " + std::to_string(id);
                        net.Send(msg, true);
                    }
                };

                drawButton({ 270, 160, 360, 50 }, "Extra Speed", "Increase movement speed permanently", 1, BLUE);
                drawButton({ 270, 220, 360, 50 }, "Extra Ammo", "Gain larger ammo capacity", 2, ORANGE);
                drawButton({ 270, 280, 360, 50 }, "Time Developer", "Shorter ability cooldowns", 3, PURPLE);
                drawButton({ 270, 340, 360, 50 }, "Freezer", "Freeze all enemies for 1.5s", 4, PINK);
            }
            else {
                if (IsKeyDown(KEY_W)) net.Send("W", false);
                if (IsKeyDown(KEY_S)) net.Send("S", false);
                if (IsKeyDown(KEY_A)) net.Send("A", false);
                if (IsKeyDown(KEY_D)) net.Send("D", false);

                for (auto& pl : players) {
                    if (std::strcmp(pl.name, myName.c_str()) == 0) {
                        DrawRectangle((int)pl.posX, (int)pl.posY, 40, 40, GREEN);
                        DrawText(pl.name, (int)pl.posX, (int)pl.posY - 20, 14, BLACK);
                    }
                    else {
                        Color col;

                        if (pl.power == 0) {
                            col = { 211, 211, 211, 180 }; // Ghost → light brown
                            DrawRectangle((int)pl.posX, (int)pl.posY, 40, 40, col);
                            continue;
                        }
                        else if (pl.power == 1) col = BLUE;      // Extra Speed
                        else if (pl.power == 2) col = ORANGE;    // Extra Ammo
                        else if (pl.power == 3) col = PURPLE;    // Time Developer
                        else if (pl.power == 4) col = PINK;     // Freezer
                        else col = RED;                          // fallback

                        DrawRectangle((int)pl.posX, (int)pl.posY, 40, 40, col);
                        DrawText(pl.name, (int)pl.posX, (int)pl.posY - 20, 14, BLACK);
                    }
                }
            }

            DrawFPS(10, 10);
        }

        EndDrawing();
    }

    net.Disconnect();
    CloseWindow();
    return 0;
}