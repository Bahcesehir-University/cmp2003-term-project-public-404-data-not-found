#include "analyzer.h"
#include <iostream>
#include <fstream>
#include <string_view>
#include <charconv>
#include <algorithm>

static string_view trim(string_view s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (string_view::npos == first) 
        return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    
    return s.substr(first, (last - first + 1));
}

static bool parseCSVHelper(const string& line, string& zoneId,int& hour) {
    if (line.empty()) 
        return false;
    if (line.find("TripID") != string::npos) 
        return false;

    int commaPositions[5];
    int commaCount = 0;

    for (int i = 0; i < (int)line.length() && commaCount < 5; ++i) {
        if (line[i] == ',') commaPositions[commaCount++] = i;
    }

    if (commaCount < 5) 
        return false;

    string_view sv(line);

    string_view zone_str = trim(sv.substr(commaPositions[0] + 1,
        commaPositions[1] - commaPositions[0] - 1));
    if (zone_str.empty()) 
        return false;

    string_view date_time_str = trim(sv.substr(commaPositions[2] + 1,
        commaPositions[3] - commaPositions[2] - 1));
    if (date_time_str.empty()) 
        return false;

    size_t spacePos =date_time_str.find(' ');
    if (spacePos == string_view::npos || spacePos + 1 >= date_time_str.length())
        return false;

    int h = 0;
    const char* start_ptr = date_time_str.data() + spacePos + 1;
    const char* end_ptr = date_time_str.data() + date_time_str.length();

    auto res = from_chars(start_ptr, end_ptr, h);
    if (res.ec != errc() || h < 0 || h > 23)
        return false;

    zoneId = string(zone_str);
    hour = h;
    
    return true;
}

bool TripAnalyzer::parseCSV(const string& line, string& zoneId, int& hour) const {
    return parseCSVHelper(line,zoneId,hour);
}

void TripAnalyzer::ingestFile(const string& csvPath) {
    ifstream file(csvPath);
    if (!file.is_open()) 
        return;

    const size_t BUFFER_SIZE = 128 * 1024;
    char buffer[BUFFER_SIZE];
    file.rdbuf()->pubsetbuf(buffer, BUFFER_SIZE);

    string line;
    line.reserve(128);

    string zone;
    int hour;

    while (getline(file, line)) {
        if (parseCSV(line, zone, hour)) {
            zone_counts[zone]++;
            slot_counts[zone][hour]++;
        }
    }
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    if (k <= 0) 
        return {};
    vector<ZoneCount> result;
    result.reserve(zone_counts.size());

    for (const auto& kv : zone_counts)
        result.push_back({ kv.first, (long long)kv.second });

    auto cmp = [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) 
        {
            return a.count > b.count;
        }
        return a.zone < b.zone;
    };

    if ((size_t)k < result.size()) {
        partial_sort(result.begin(), result.begin() + k, result.end(), cmp);
        result.resize(k);
    } 
    else {
        sort(result.begin(),result.end(), cmp);
    }

    return result;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    if (k <= 0) return {};
    vector<SlotCount> result;

    for (const auto& zoneEntry : slot_counts) 
    {
        for (const auto& hEntry : zoneEntry.second) {
            result.push_back({ zoneEntry.first, hEntry.first, (long long)hEntry.second });
        }
    }

    auto cmp = [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count)
            return a.count > b.count;
        
        if (a.zone != b.zone) 
        {
            return a.zone < b.zone;
        }
        return a.hour < b.hour;
    };

    if ((size_t)k < result.size()) {
        partial_sort(result.begin(), result.begin() + k, result.end(), cmp);
        result.resize(k);
    }
    else 
    {
        sort(result.begin(), result.end(), cmp);
    }

    return result;
}

