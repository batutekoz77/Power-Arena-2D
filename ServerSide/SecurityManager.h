#pragma once
#include <unordered_map>
#include <string>
#include <chrono>
#include <vector>

struct IPBanInfo {
    int strikes = 0;
    bool permanent = false;
    std::chrono::steady_clock::time_point banUntil;
};

class SecurityManager {
public:
    static SecurityManager& getInstance();

    bool isBlacklisted(const std::string& ip);
    void recordConnectAttempt(const std::string& ip);

    void loadPermanentBans(const std::string& filename = "permanent_bans.json");
    void savePermanentBans(const std::string& filename = "permanent_bans.json");

    // log dosyasına yazmak için
    void logToFile(const std::string& text, const std::string& filename = "security.log");

private:
    SecurityManager() = default;
    SecurityManager(const SecurityManager&) = delete;
    SecurityManager& operator=(const SecurityManager&) = delete;

    void blacklistIP(const std::string& ip);
    void addPermanentBan(const std::string& ip, const std::string& reason);

    std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> requestLog;
    std::unordered_map<std::string, IPBanInfo> banInfo;

    struct PermanentBanEntry {
        std::string ip;
        std::string reason;
    };
    std::vector<PermanentBanEntry> permanentBans;

    const int MAX_ATTEMPTS = 10;
    const int TIME_WINDOW_SEC = 15;
    const int TEMP_BAN_SECONDS = 180;
};