// UserStats.h
#ifndef USERSTATS_H
#define USERSTATS_H

#include <map>
#include <string>
#include <mutex>
#include <cstdint>

extern std::map<std::string, uint64_t> userDifficultyTotal;
extern std::map<std::string, uint64_t> userAcceptedShares;
extern std::mutex userStatsMutex;

void saveUserStats();
void loadUserStats();

#endif