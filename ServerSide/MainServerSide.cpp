#include "SecurityManager.h"
#include "NetworkManager.h"

int main() {
    auto& security = SecurityManager::getInstance();
    security.loadPermanentBans();

    NetworkManager server;
    server.Run();
    return 0;
}