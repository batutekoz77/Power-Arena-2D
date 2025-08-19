#include "SecurityManager.h"
#include "NetworkManager.h"

int main() {
    auto& security = SecurityManager::getInstance();
    security.loadPermanentBans();

    NetworkManager server;
    server.Run();
    return 0;
}

/*
git add .
git commit -m ""
git push
*/