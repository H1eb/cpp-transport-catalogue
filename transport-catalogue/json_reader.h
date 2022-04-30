#pragma once

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "transport_router.h"

struct DBQueries {
    std::deque<Stop> stops;
    std::deque<Bus> buses;
    std::deque<Stat> queries;

    RenderSettings render_settings;
    RoutingSettings routing_settings;
};

json::Document LoadJSON(std::istream& input);
DBQueries ParseJson(const json::Document& document);

svg::Color SetColor(const json::Node& node);

json::Node Generate_Error_Message(int id, std::string_view text);

json::Array GetAnswer(std::deque<Stat> queries, RequestHandler requestHandler);
json::Node GetTransportMap(const Stat& stat, const RequestHandler& rh);
json::Node GetBusesList(const Stat& stat, const RequestHandler& rh);
json::Node GetBusInfo(const Stat& stat, const RequestHandler& rh);
json::Node GetRouteInfo(const Stat& stat, const RequestHandler& rh);