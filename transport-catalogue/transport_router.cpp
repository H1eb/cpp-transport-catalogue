#include "transport_router.h"

using namespace std;

void TransportRouter::DistanceToEdge(map_pair_distance pair_distance){
    for (const auto& [stops_indexes, props] : pair_distance) {

        double travel_time = double(props.distance) / routing_settings_.bus_velocity + double(routing_settings_.bus_wait_time);
        graph::EdgeId id = routes_graph_.AddEdge({ stops_indexes.first, stops_indexes.second, travel_time });
        EdgeProps edge_prop(props);
        edge_prop.travel_time = travel_time;
        edgeID_n_edge_props_.emplace(id, std::move(edge_prop));
    }
}

int TransportRouter::GetDistanceFromTo(const Stop* prev_stop, const Stop* current_stop, graph::VertexId vid_stop_from, graph::VertexId prev_stop_to) {
    
    auto distance_prev_current = transport_catalogue_.GetDistanceBetweenStops(prev_stop, current_stop); 
    int distance = pair_n_distance_.at({ vid_stop_from, prev_stop_to }) + distance_prev_current.value();
    
    return distance;
}

void TransportRouter::CreateGraph() {
    map_pair_distance pair_distance;

    graph::VertexId vid_stop_from = 0;
    graph::VertexId prev_stop_to = 0;
    graph::VertexId current_stop_to = 0;

    for (const Bus& bus : transport_catalogue_.GetBuses()) {
        for (auto it_outer = bus.stops.begin(); it_outer + 1 != bus.stops.end(); ++it_outer) {
            int span_count = 0;
            const Stop* stop_from = transport_catalogue_.GetStopByName(*it_outer);
            vid_stop_from = transport_catalogue_.GetStopIndex(stop_from);
            int zero_stop_distance = 0;
            pair_n_distance_[{ vid_stop_from, vid_stop_from }] = zero_stop_distance;
            for (auto it_inner = it_outer + 1; it_inner != bus.stops.end(); ++it_inner) {
                ++span_count;
                const Stop* prev_stop = transport_catalogue_.GetStopByName(*std::prev(it_inner));
                const Stop* current_stop = transport_catalogue_.GetStopByName(*it_inner);
                prev_stop_to = transport_catalogue_.GetStopIndex(prev_stop);
                current_stop_to = transport_catalogue_.GetStopIndex(current_stop);
                
                int distance_from_to_current = GetDistanceFromTo(prev_stop, current_stop, vid_stop_from, prev_stop_to); 
                
                pair_n_distance_[{ vid_stop_from, current_stop_to }] = distance_from_to_current;
                EdgeProps edge_prop;
                edge_prop.bus = &bus;
                edge_prop.span_count = span_count;
                edge_prop.distance = distance_from_to_current;
                edge_prop.travel_time = 0.0;
                edge_prop.stop_from = *it_outer;
                
                auto it_find = pair_distance.find({ vid_stop_from, current_stop_to });

                if (it_find == pair_distance.end()) {
                    pair_distance[{ vid_stop_from, current_stop_to }] = edge_prop;
                } else if (it_find->second.distance > distance_from_to_current) {
                    pair_distance[{ vid_stop_from, current_stop_to }] = edge_prop;
                }

                if (!bus.is_roundtrip) {
                    
                    int distance_current_to_from = GetDistanceFromTo(current_stop, prev_stop, prev_stop_to, vid_stop_from); 
                    
                    pair_n_distance_[{ current_stop_to, vid_stop_from }] = distance_current_to_from;
                    
                    EdgeProps edge_prop_rev(edge_prop);
                    edge_prop_rev.distance = distance_current_to_from;
                    edge_prop_rev.stop_from = *it_inner;
                    
                    auto it_find_rev = pair_distance.find({ current_stop_to, vid_stop_from });

                    if (it_find_rev == pair_distance.end()) {
                        pair_distance[{ current_stop_to, vid_stop_from }] = edge_prop_rev;
                    } else if (it_find_rev->second.distance > distance_current_to_from) {
                        pair_distance[{ current_stop_to, vid_stop_from }] = edge_prop_rev;
                    }
                }
            }
        }

        DistanceToEdge(pair_distance);
        
    }
}

const EdgeProps& TransportRouter::GetEdgeProps(graph::EdgeId id) const {
    return edgeID_n_edge_props_.at(id);
}

const RoutingSettings& TransportRouter::GetRouterSettings() const {
    return routing_settings_;
}

const RouteStat TransportRouter::GetRoute(RoutingSettings routing_settings, std::optional<graph::Router<double>::RouteInfo> route_info, const TransportRouter& transport_router) const {

    RouteStat route_stat;
    const std::vector<graph::EdgeId> edges = route_info.value().edges;
    double total_time = 0.0;

    if (edges.empty()) {
        RouteStat empty_route;
        empty_route.total_time = 0.0;
        return empty_route;
    }

    for (const auto& edgeID : edges) {
        EdgeProps props = transport_router.GetEdgeProps(edgeID);
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
