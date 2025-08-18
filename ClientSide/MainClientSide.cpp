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
            if (IsKeyDown(KEY_W)) net.Send("W", false);
            if (IsKeyDown(KEY_S)) net.Send("S", false);
            if (IsKeyDown(KEY_A)) net.Send("A", false);
            if (IsKeyDown(KEY_D)) net.Send("D", false);

            for (auto& pl : players) {
                Color col = (std::strcmp(pl.name, myName.c_str()) == 0) ? GREEN : RED;
                DrawRectangle((int)pl.posX, (int)pl.posY, 40, 40, col);
                DrawText(pl.name, (int)pl.posX, (int)pl.posY - 20, 14, BLACK);
            }

            DrawFPS(10, 10);
        }

        EndDrawing();
    }

    net.Disconnect();
    CloseWindow();
    return 0;
}