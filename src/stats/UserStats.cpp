#include "UserStats.h"
#include <fstream>
#include <iostream>  // For printing to the console
#include <map>
#include <string>
#include <mutex>
#include <cstdint>

std::map<std::string, uint64_t> userDifficultyTotal;
std::map<std::string, uint64_t> userAcceptedShares;
std::mutex userStatsMutex;

void saveUserStats() {
    std::cout << "Saving user stats to user_stats.txt..." << std::endl;

    std::ofstream file("user_stats.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open user_stats.txt for writing." << std::endl;
        return;
    }

    std::unique_lock<std::mutex> lock(userStatsMutex);

    // Use structured format: LOGIN|DIFFICULTY|SHARES
    file << "# XMRig Proxy User Stats - Format: LOGIN|DIFFICULTY|ACCEPTED_SHARES\n";

    // Combine both maps - iterate through difficulty map and match with shares
    for (const auto& diffPair : userDifficultyTotal) {
        const std::string& login = diffPair.first;
        uint64_t difficulty = diffPair.second;

        // Find corresponding shares (default to 0 if not found)
        uint64_t shares = 0;
        auto sharesIt = userAcceptedShares.find(login);
        if (sharesIt != userAcceptedShares.end()) {
            shares = sharesIt->second;
        }

        file << login << "|" << difficulty << "|" << shares << "\n";
    }

    // Also save any users who have shares but no difficulty (edge case)
    for (const auto& sharesPair : userAcceptedShares) {
        const std::string& login = sharesPair.first;

        // Skip if already written above
        if (userDifficultyTotal.find(login) != userDifficultyTotal.end()) {
            continue;
        }

        file << login << "|0|" << sharesPair.second << "\n";
    }

    lock.unlock();
    file.close();

    std::cout << "User stats saved successfully." << std::endl;
}

void loadUserStats() {
    std::cout << "Attempting to load user stats from user_stats.txt..." << std::endl;
    std::ifstream file("user_stats.txt");
    if (!file.is_open()) {
        std::cout << "user_stats.txt not found. Starting with empty stats." << std::endl;
        return;
    }

    std::unique_lock<std::mutex> lock(userStatsMutex);

    userDifficultyTotal.clear();
    userAcceptedShares.clear();

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse format: LOGIN|DIFFICULTY|SHARES
        size_t firstPipe = line.find('|');
        size_t secondPipe = line.find('|', firstPipe + 1);

        if (firstPipe == std::string::npos || secondPipe == std::string::npos) {
            std::cerr << "Warning: Invalid format on line " << lineNum << ": " << line << std::endl;
            continue;
        }

        try {
            std::string login = line.substr(0, firstPipe);
            std::string diffStr = line.substr(firstPipe + 1, secondPipe - firstPipe - 1);
            std::string sharesStr = line.substr(secondPipe + 1);

            uint64_t difficulty = std::stoull(diffStr);
            uint64_t shares = std::stoull(sharesStr);

            userDifficultyTotal[login] = difficulty;
            userAcceptedShares[login] = shares;

        }
        catch (const std::exception& e) {
            std::cerr << "Warning: Error parsing line " << lineNum << ": " << e.what() << std::endl;
            continue;
        }
    }

    lock.unlock();
    file.close();

    std::cout << "User stats loaded successfully. Loaded " << userDifficultyTotal.size() << " user records." << std::endl;
}