#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
using namespace std;

struct ZoneCount {
    string zone;
    long long count;
};

struct SlotCount {
    string zone;
    int hour; // 0-23
    long long count;
};

class TripAnalyzer {

private:
    unordered_map<string, int64_t> zone_counts;
    unordered_map<string, unordered_map<int, int64_t>> slot_counts;

    bool parseCSV(const string& line, string&  zoneId, int& hour) const;

public:
    void ingestFile(const string& csvPath);
    vector<ZoneCount> topZones(int k = 10) const;
    vector<SlotCount> topBusySlots(int k = 10) const;
};
