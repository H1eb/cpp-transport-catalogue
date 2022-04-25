#pragma once

#include "geo.h"

#include <array>
#include <deque>
#include <set>
#include <string>
#include <unordered_map>

struct Stop {
    std::string name;
    geo::Coordinates position;
    std::deque<std::pair<std::string, int>> road_distances;
};

struct Bus {
    std::string name;
    std::deque<std::string> stops;
    bool is_roundtrip;
};

enum class RequestType {
    BUS,
    STOP,
    MAP,
    ROUTE
};  

struct Stat {
    int id;
    RequestType type;
    std::unordered_map<std::string, std::string> key_values;
};

struct BusStat {
    std::string bus_name;
    int stops_count = 0;
    int unique_stops = 0;
    double length = 0.0;
    double curvature = 0.0;
};

struct BusesToStop {
    std::string stop_name;
    bool notFound = true;
    std::set<std::string_view> buses;
};
