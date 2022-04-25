#pragma once

#include "domain.h"
#include "geo.h"

#include <algorithm>
#include <deque>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace tc {

    struct HasherForPair {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2>& ab) const {
            return std::hash<T1>()(ab.first) + 37 * std::hash<T2>()(ab.second);
        }
    };

    class TransportCatalogue {
    public:
        void AddBus(const Bus& bus);
        void AddStop(const Stop& stop);
        void SetDistance(const Stop& stop);
        void FillTransportBase(const std::deque<Stop>& stops, const std::deque<Bus>& buses);
        const Bus* GetRouteByBusName(std::string_view bus_name) const;
        const Stop* GetStopByName(std::string_view stop_name) const;
        const std::set<std::string_view>& GetBusesToStop(const Stop* stop) const;
        std::optional<int> GetDistanceBetweenStops(const Stop* stop_from, const Stop* stop_to) const;
        const std::deque<Stop>& GetStops() const;
        const std::deque<Bus>& GetBuses() const;
        size_t GetAllStopsCount() const;
        size_t GetStopIndex(const Stop* stop) const;

    private:
        std::unordered_map<const Stop*, size_t> stops_index;
        std::deque<Stop> stops_;
        std::deque<Bus> buses_;

        std::unordered_map<std::string_view, const Stop*> names_stops_;
        std::unordered_map<std::string_view, const Bus*> names_buses_;

        std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, HasherForPair> stops_distance_;
    };
}