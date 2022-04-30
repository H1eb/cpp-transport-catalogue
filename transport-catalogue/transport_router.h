#pragma once

#include "router.h"
#include "transport_catalogue.h"

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

struct RouteElement {
    std::string type;
    std::string stop_name;
    std::string bus_name;
    double time;
    int span_count;
};

struct RouteStat {
    int id;
    double total_time = 0.0;
    std::deque<RouteElement> items;
    double bus_wait_time;
};

struct EdgeProps {
    const Bus* bus;
    int span_count;
    int distance;
    double travel_time;
    std::string_view stop_from;
};

using map_pair_distance = std::unordered_map<std::pair<graph::VertexId, graph::VertexId>, EdgeProps, tc::HasherForPair>;

class TransportRouter {

public:
    TransportRouter(graph::DirectedWeightedGraph<double>& routes_graph, 
                     const tc::TransportCatalogue& transport_catalogue, 
                     const RoutingSettings& routing_settings)
        : routes_graph_(routes_graph)
        , transport_catalogue_(transport_catalogue)
        , routing_settings_(routing_settings) {
    }

    void CreateGraph();

    const EdgeProps& GetEdgeProps(graph::EdgeId) const;
    const RoutingSettings& GetRouterSettings() const;
    int GetDistanceFromTo(const Stop* prev_stop, const Stop* current_stop, graph::VertexId vid_stop_from, graph::VertexId prev_stop_to);
    void DistanceToEdge(map_pair_distance pair_distance);
    
    
    const RouteStat GetRoute(RoutingSettings routing_settings, std::optional<graph::Router<double>::RouteInfo> route_info, const TransportRouter& transport_router) const;

private:
    graph::DirectedWeightedGraph<double>& routes_graph_;
    const tc::TransportCatalogue& transport_catalogue_;
    const RoutingSettings& routing_settings_;
    std::unordered_map<graph::EdgeId, EdgeProps> edgeID_n_edge_props_;
    std::unordered_map<std::pair<graph::VertexId, graph::VertexId>, int, tc::HasherForPair> pair_n_distance_;
};

