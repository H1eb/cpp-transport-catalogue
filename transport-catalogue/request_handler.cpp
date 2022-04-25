#include "request_handler.h"

#include <cmath>

RequestHandler::RequestHandler(const tc::TransportCatalogue& transport_catalogue, const MapRenderer& renderer, const graph::Router<double>& router, const TransportRouter& transport_router)
    : transport_catalogue_(transport_catalogue)
    , renderer_(renderer)
    , router_(router)
    , transport_router_(transport_router) {

    for (const Stop& stop : transport_catalogue_.GetStops()) {
        sorted_unique_stopnames_.insert(stop.name);
    }
    for (const Bus& bus : transport_catalogue_.GetBuses()) {
        sorted_buses_.push_back(&bus);
    }
    std::sort(sorted_buses_.begin(), sorted_buses_.end(), [](const Bus* lhs, const Bus* rhs) {
        return lhs->name < rhs->name;
    });
}

const BusStat RequestHandler::GetBusStat(const std::string_view bus_name) const {
    BusStat bus_route;
    bus_route.bus_name = bus_name;

    const Bus* bus = transport_catalogue_.GetRouteByBusName(bus_name);
    if (bus == nullptr) {
        return bus_route;
    }

    bus_route.stops_count = static_cast<int>(bus->stops.size());

    if (!bus->is_roundtrip) {
        bus_route.stops_count = 2 * bus_route.stops_count - 1;
    }

    bus_route.unique_stops = GetUniqueStopsCount(bus);
    bus_route.length = CalculateRealLength(bus);
    double gps_length = CalculateGPSLength(bus);
    constexpr double EPSILON = 1e-6;

    if (std::abs(gps_length) < EPSILON) {
        bus_route.curvature = 0.0;
    } else {
        bus_route.curvature = bus_route.length / gps_length;
    }

    if (std::isnan(bus_route.curvature)) {
        bus_route.curvature = 0.0;
    }

    return bus_route;
}

const BusesToStop RequestHandler::GetBusesByStop(const std::string_view stop_name) const {
    BusesToStop bus;
    bus.stop_name = stop_name;

    const Stop* stop = transport_catalogue_.GetStopByName(stop_name);
    if (stop == nullptr) {
        bus.notFound = true;
        return bus;
    } else {
        bus.notFound = false;
    }

    bus.buses = transport_catalogue_.GetBusesToStop(stop);
    return bus;
}

svg::Document RequestHandler::RenderMap() const {
    svg::Document doc;
    std::deque<geo::Coordinates> geo_points;

    for (std::string_view stop_name : sorted_unique_stopnames_) {
        if (transport_catalogue_.GetBusesToStop(transport_catalogue_.GetStopByName(stop_name)).empty()) {
            continue;
        }

        double lat = transport_catalogue_.GetStopByName(stop_name)->position.lat;
        double lng = transport_catalogue_.GetStopByName(stop_name)->position.lng;
        geo_points.push_back({ lat, lng });
    }
    
    SphereProjector sp(geo_points.begin(), geo_points.end(), renderer_.GetWidth(), renderer_.GetHeight(), renderer_.GetPadding());
    const size_t colors_in_palete = renderer_.GetColor();
    size_t color_idx = 0;

    for (const Bus* bus_ptr : sorted_buses_) {

        std::deque<svg::Point> stops_points;

        for (std::string_view stop : bus_ptr->stops) {
            double lat = transport_catalogue_.GetStopByName(stop)->position.lat;
            double lng = transport_catalogue_.GetStopByName(stop)->position.lng;
            stops_points.push_back(sp({ lat, lng }));
        }

        if (!bus_ptr->is_roundtrip) {
            auto it = bus_ptr->stops.rbegin() + 1;

            while (it != bus_ptr->stops.rend()) {
                double lat = transport_catalogue_.GetStopByName(*it)->position.lat;
                double lng = transport_catalogue_.GetStopByName(*it)->position.lng;
                stops_points.push_back(sp({ lat, lng }));
                ++it;
            }
        }

        renderer_.AddBusRoutes(doc, stops_points, color_idx);
        ++color_idx;

        if (color_idx == colors_in_palete) {
            color_idx = 0;
        }
    }

    color_idx = 0;
    for (const Bus* bus_ptr : sorted_buses_) {

        std::string_view first_stop = bus_ptr->stops.front();
        double lat = transport_catalogue_.GetStopByName(first_stop)->position.lat;
        double lng = transport_catalogue_.GetStopByName(first_stop)->position.lng;
        renderer_.AddBusNames(doc, sp({ lat, lng }), bus_ptr->name, color_idx);
        std::string_view last_stop = bus_ptr->stops.back();
        bool is_same_first_last_stops = (first_stop == last_stop);
        if (!bus_ptr->is_roundtrip && !is_same_first_last_stops) {
            lat = transport_catalogue_.GetStopByName(last_stop)->position.lat;
            lng = transport_catalogue_.GetStopByName(last_stop)->position.lng;
            renderer_.AddBusNames(doc, sp({ lat, lng }), bus_ptr->name, color_idx);
        }

        ++color_idx;

        if (color_idx == colors_in_palete) {
            color_idx = 0;
        }
    }

    std::deque<std::pair<svg::Point, std::string_view>> stops_w_buses_points;
    for (std::string_view stop_name : sorted_unique_stopnames_) {
        if (transport_catalogue_.GetBusesToStop(transport_catalogue_.GetStopByName(stop_name)).empty()) {
            continue;
        }

        double lat = transport_catalogue_.GetStopByName(stop_name)->position.lat;
        double lng = transport_catalogue_.GetStopByName(stop_name)->position.lng;
        stops_w_buses_points.push_back(std::make_pair(sp({ lat, lng }), stop_name));
    }

    renderer_.AddStopPoints(doc, stops_w_buses_points);
    renderer_.AddStopNames(doc, stops_w_buses_points);
    return doc;
}

// ----Auxiliary private methods----
double RequestHandler::CalculateGPSLength(const Bus* bus) const {

    const auto& stops = bus->stops;
    double direct_length = 0.0;
    for (auto it = stops.begin(); it + 1 != stops.end(); ++it) {
        auto* stop_prev = transport_catalogue_.GetStopByName(*it);
        auto* stop_next = transport_catalogue_.GetStopByName(*(std::next(it)));
        direct_length += geo::ComputeDistance({ stop_prev->position.lat, stop_prev->position.lng }, { stop_next->position.lat, stop_next->position.lng });
    }

    if (!bus->is_roundtrip) {
        return 2 * direct_length;
    }

    return direct_length;
}

int RequestHandler::CalculateRealLength(const Bus* bus) const {
    const auto& stops = bus->stops;
    int length = 0.0;
    for (auto it = stops.begin(); it + 1 != stops.end(); ++it) {
        auto* stop_prev = transport_catalogue_.GetStopByName(*it);
        auto* stop_next = transport_catalogue_.GetStopByName(*(std::next(it)));
        auto distance_prev_next = transport_catalogue_.GetDistanceBetweenStops(stop_prev, stop_next);
        auto distance_next_prev = transport_catalogue_.GetDistanceBetweenStops(stop_next, stop_prev);

        if (bus->is_roundtrip) {
            length += distance_prev_next.value();
        } else {
            length += distance_prev_next.value() + distance_next_prev.value();
        }
    }
    return length;
}

int RequestHandler::GetUniqueStopsCount(const Bus* bus) const {
    std::unordered_set<std::string_view> unique_stops(bus->stops.begin(), bus->stops.end());

    return static_cast<int>(unique_stops.size());
}

const std::optional<RouteStat> RequestHandler::GetRoute(std::string_view from, std::string_view to) const {
    const RoutingSettings routing_settings = transport_router_.GetRouterSettings();
    const Stop* stop_from = transport_catalogue_.GetStopByName(from);
    const Stop* stop_to = transport_catalogue_.GetStopByName(to);
    graph::VertexId idx_stop_from = transport_catalogue_.GetStopIndex(stop_from);
    graph::VertexId idx_stop_to = transport_catalogue_.GetStopIndex(stop_to);
    std::optional<graph::Router<double>::RouteInfo> route_info = router_.BuildRoute(idx_stop_from, idx_stop_to);

    if (route_info == std::nullopt) {
        return std::nullopt;
    }

    RouteStat route_stat;
    const std::vector<graph::EdgeId> edges = route_info.value().edges;
    double total_time = 0.0;

    if (edges.empty()) {
        RouteStat empty_route;
        empty_route.total_time = 0.0;
        return empty_route;
    }

    for (const auto& edgeID : edges) {
        EdgeProps props = transport_router_.GetEdgeProps(edgeID);
        RouteElement wait_element;
        wait_element.stop_name = props.stop_from;
        wait_element.time = routing_settings.bus_wait_time;
        wait_element.type = "Wait"s;
        route_stat.items.push_back(std::move(wait_element));
        RouteElement go_element;
        go_element.time = props.travel_time - routing_settings.bus_wait_time;
        go_element.type = "Bus"s;
        go_element.bus_name = props.bus->name;
        go_element.span_count = props.span_count;
        route_stat.items.push_back(std::move(go_element));
        total_time += props.travel_time;
    }

    route_stat.total_time = total_time;
    route_stat.bus_wait_time = routing_settings.bus_wait_time;
    return route_stat;
}
