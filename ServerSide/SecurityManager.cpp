#include "SecurityManager.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>

SecurityManager& SecurityManager::getInstance() {
    static SecurityManager instance;
    return instance;
}

bool SecurityManager::isBlacklisted(const std::string& ip) {
    auto it = banInfo.find(ip);
    if (it == banInfo.end()) return false;

    auto& info = it->second;

    if (info.permanent) {
        return true;
    }

    auto now = std::chrono::steady_clock::now();
    if (now >= info.banUntil) {
        return false;
    }

    return true;
}

void SecurityManager::recordConnectAttempt(const std::string& ip) {
    auto now = std::chrono::steady_clock::now();

    auto& attempts = requestLog[ip];
    attempts.push_back(now);

    auto cutoff = now - std::chrono::seconds(TIME_WINDOW_SEC);
    attempts.erase(std::remove_if(attempts.begin(), attempts.end(),
        [&](auto& t) { return t < cutoff; }),
        attempts.end());

    if ((int)attempts.size() > MAX_ATTEMPTS) {
        blacklistIP(ip);
    }
}

void SecurityManager::blacklistIP(const std::string& ip) {
    auto& info = banInfo[ip];
    info.strikes++;

    if (info.strikes >= 3) {
        info.permanent = true;
        addPermanentBan(ip, "Exceeded connection attempts 3 times");
        std::cout << "[SECURITY] Permanently blacklisted IP: " << ip << std::endl;
        logToFile("[SECURITY] Permanently blacklisted IP: " + ip);
    }
    else {
        info.banUntil = std::chrono::steady_clock::now() + std::chrono::seconds(TEMP_BAN_SECONDS);
        std::cout << "[SECURITY] Temporarily blacklisted IP ("
            << TEMP_BAN_SECONDS << "s): " << ip
            << " [Strike " << info.strikes << "]" << std::endl;

        logToFile("[SECURITY] Temporarily blacklisted IP (" +
            std::to_string(TEMP_BAN_SECONDS) + "s): " + ip +
            " [Strike " + std::to_string(info.strikes) + "]");
    }
}

void SecurityManager::addPermanentBan(const std::string& ip, const std::string& reason) {
    permanentBans.push_back({ ip, reason });
    savePermanentBans();
}

void SecurityManager::savePermanentBans(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << "{\n  \"blacklisted_ips\": [\n";
    for (size_t i = 0; i < permanentBans.size(); i++) {
        file << "    {\n";
        file << "      \"ip\": \"" << permanentBans[i].ip << "\",\n";
        file << "      \"reason\": \"" << permanentBans[i].reason << "\"\n";
        file << "    }";
        if (i + 1 < permanentBans.size()) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";

    file.close();
}

void SecurityManager::loadPermanentBans(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    size_t pos = 0;
    while ((pos = content.find("\"ip\":", pos)) != std::string::npos) {
        size_t start = content.find("\"", pos + 5) + 1;
        size_t end = content.find("\"", start);
        std::string ip = content.substr(start, end - start);

        pos = content.find("\"reason\":", end);
        start = content.find("\"", pos + 8) + 1;
        end = content.find("\"", start);
        std::string reason = content.substr(start, end - start);

        permanentBans.push_back({ ip, reason });
        banInfo[ip].permanent = true;

        pos = end;
    }
}

void SecurityManager::logToFile(const std::string& text, const std::string& filename) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) return;

    std::time_t now = std::time(nullptr);
    char timeStr[64];

    struct tm tstruct;
    localtime_s(&tstruct, &now);
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &tstruct);

    file << "[" << timeStr << "] " << text << "\n";
    file.close();
}