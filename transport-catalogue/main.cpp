#include "graph.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"

using namespace std;

int main() {
    tc::TransportCatalogue transport_catalogue;

    DBQueries dbq = ParseJson(LoadJSON(cin));

    transport_catalogue.FillTransportBase(dbq.stops, dbq.buses);

    graph::DirectedWeightedGraph<double> routes_graph(transport_catalogue.GetAllStopsCount());
    
    TransportRouter transport_router(routes_graph, transport_catalogue, dbq.routing_settings);
    transport_router.CreateGraph();

    graph::Router<double> router(routes_graph);

    MapRenderer map_renderer(dbq.render_settings);

    RequestHandler requestHandler(transport_catalogue, map_renderer, router, transport_router);

    json::Array answer = GetAnswer(dbq.queries, requestHandler);

    json::Print(json::Document{ answer }, std::cout);

    return 0;
}
