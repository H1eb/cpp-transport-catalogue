#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <deque>
#include <optional>
#include <set>
#include <string_view>

using Container_stops_points = std::deque<std::pair<svg::Point, std::string_view>>;

class RequestHandler {
public:
    RequestHandler(const tc::TransportCatalogue& transport_catalogue, 
                   const MapRenderer& renderer, const graph::Router<double>& router, 
                   const TransportRouter& transport_router);

    const BusStat GetBusStat(const std::string_view bus_name) const;
    const BusesToStop GetBusesByStop(const std::string_view stop_name) const;
    svg::Document RenderMap() const;
    const std::optional<RouteStat> GetRoute(const std::string_view from, const std::string_view to) const;
    
    svg::Document DrawPolyline(svg::Document doc, SphereProjector sp, size_t colors_in_palete) const;
    svg::Document DrawBusName(svg::Document doc, SphereProjector sp, size_t colors_in_palete) const;
    geo::Coordinates GetLatAndLng(std::string_view stop) const;

private:
    int GetUniqueStops(const Bus* bus) const;
    double CalculateGPSLength(const Bus* bus) const;
    int CalculateRealLength(const Bus* bus) const;
    double CalculateCurvature(double bus_route_length, double gps_length) const;
    int CalculateStopCount(const Bus* bus) const;

    const tc::TransportCatalogue& transport_catalogue_;
    const MapRenderer& renderer_;
    const graph::Router<double>& router_;
    const TransportRouter& transport_router_;
    std::deque<const Bus*> sorted_buses_;
    std::set<std::string_view> sorted_unique_stopnames_;
};