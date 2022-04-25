#include "transport_catalogue.h"

using namespace std;

namespace tc {

    void TransportCatalogue::AddStop(const Stop& stop) {
        const auto& link = stops_.emplace_back(stop);
        names_stops_[link.name] = &link;
        stops_index[&link] = stops_index.size();
    }

    size_t TransportCatalogue::GetStopIndex(const Stop* stop) const {
        return stops_index.at(stop);
    }

    void TransportCatalogue::SetDistance(const Stop& stop) {
        const Stop* stop_from = GetStopByName(stop.name);

        for (const auto& distance : stop.road_distances) {
            const Stop* stop_to = GetStopByName(distance.first);
            stops_distance_.emplace(make_pair(stop_from, stop_to), distance.second);
        }
    }

    void TransportCatalogue::AddBus(const Bus& bus) {
        const auto& link = buses_.emplace_back(bus);
        names_buses_[link.name] = &link;

        for (const auto& stop_name : link.stops) {
            const Stop* stop = GetStopByName(stop_name);
            stop_to_buses_[stop].insert(link.name);
        }
    }

    void TransportCatalogue::FillTransportBase(const std::deque<Stop>& stops, const std::deque<Bus>& buses) {
        for (auto& stop : stops) {
            AddStop(stop);
        }
        for (auto& stop : stops) {
            SetDistance(stop);
        }
        for (auto& bus : buses) {
            AddBus(bus);
        }
    }

    const Bus* TransportCatalogue::GetRouteByBusName(std::string_view bus_name) const {
        auto it = names_buses_.find(bus_name);

        if (it != names_buses_.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    } 

    const Stop* TransportCatalogue::GetStopByName(std::string_view stop_name) const {
        auto it = names_stops_.find(stop_name);

        if (it != names_stops_.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    } 

    const set<string_view>& TransportCatalogue::GetBusesToStop(const Stop* stop) const {
        const static set<string_view> empty_set;

        auto it = stop_to_buses_.find(stop);

        if (it == stop_to_buses_.end()) {
            return empty_set;
        }

        return it->second;
    }

    std::optional<int> TransportCatalogue::GetDistanceBetweenStops(const Stop* stop_from, const Stop* stop_to) const {
        auto it = stops_distance_.find(std::make_pair(stop_from, stop_to));

        if (it == stops_distance_.end()) {
            auto it_reverse = stops_distance_.find(std::make_pair(stop_to, stop_from));

            if (it_reverse == stops_distance_.end()) {
                return std::nullopt;
            }

            return it_reverse->second;
        }

        return it->second;
    }

    const std::deque<Stop>& TransportCatalogue::GetStops() const {
        return stops_;
    }

    const std::deque<Bus>& TransportCatalogue::GetBuses() const {
        return buses_;
    }

    size_t TransportCatalogue::GetAllStopsCount() const {
        return stops_index.size();
    }

}