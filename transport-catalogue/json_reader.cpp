#include "json_reader.h"

using namespace std::literals;

json::Document LoadJSON(std::istream& input) {
    return json::Load(input);
}

json::Node Generate_Error_Message_Dict(int id) {
    return json::Builder()
        .StartDict()
        .Key("request_id"s)
        .Value(id)
        .Key("error_message"s)
        .Value("not found")
        .EndDict()
        .Build();
}

json::Array GetAnswer(std::deque<Stat> queries, RequestHandler requestHandler) {
    json::Array array;
    for (const auto& request: queries) {
        switch (request.type)
        {
        case RequestType::ROUTE:
            array.push_back(std::move(GetRoute(request, requestHandler)));
            break;
        case RequestType::BUS:
            array.push_back(std::move(GetBusInfo(request, requestHandler)));
            break;        
        case RequestType::STOP:
            array.push_back(std::move(GetBusesList(request, requestHandler)));
            break;
        case RequestType::MAP:
            array.push_back(std::move(GetTransportMap(request, requestHandler)));
            break;                    
        default:
            break;
        }
    }
    return array;
}

json::Node GetTransportMap(const Stat& stat, const RequestHandler& rh) {
    svg::Document doc_map = rh.RenderMap();

    std::stringstream stream;
    doc_map.Render(stream);

    return json::Builder()
        .StartDict()
        .Key("request_id"s)
        .Value(stat.id)
        .Key("map"s)
        .Value(std::string(stream.str()))
        .EndDict()
        .Build();
}

json::Node GetBusesList(const Stat& stat, const RequestHandler& rh) {
    const BusesToStop& stop_info = rh.GetBusesByStop(stat.key_values.at("name"s));

    if (!stop_info.notFound) {
        json::Array buses;
        for (const auto& bus : stop_info.buses)
            buses.push_back(std::string(bus));
        
        return json::Builder()
            .StartDict()
            .Key("buses"s)
            .Value(std::move(buses))
            .Key("request_id"s)
            .Value(stat.id)
            .EndDict()
            .Build();
    } else {
        return Generate_Error_Message_Dict(stat.id);
    }
}

json::Node GetBusInfo(const Stat& stat, const RequestHandler& rh) {
    const BusStat& bus_info = rh.GetBusStat(stat.key_values.at("name"s));

    if (bus_info.stops_count != 0) {
        return json::Builder()
            .StartDict()
            .Key("curvature"s)
            .Value(bus_info.curvature)
            .Key("request_id"s)
            .Value(stat.id)
            .Key("route_length"s)
            .Value(bus_info.length)
            .Key("stop_count"s)
            .Value(bus_info.stops_count)
            .Key("unique_stop_count"s)
            .Value(bus_info.unique_stops)
            .EndDict()
            .Build();
    } else {
        return Generate_Error_Message_Dict(stat.id);
    }
}

json::Node GetRoute(const Stat& stat, const RequestHandler& rh) {

    std::string from = stat.key_values.at("from");
    std::string to = stat.key_values.at("to");

    std::optional<RouteStat> route_stat_opt = rh.GetRoute(from, to);

    if (route_stat_opt != std::nullopt) {
        json::Array array;
        auto route_stat = route_stat_opt.value();
        for (const auto& item : route_stat.items) {
            if (item.type == "Wait"s) {
                json::Dict dict;

                dict.emplace("type"s, "Wait"s);
                dict.emplace("stop_name"s, item.stop_name);
                dict.emplace("time"s, route_stat.bus_wait_time);

                array.push_back(std::move(dict));
            }

            if (item.type == "Bus"s) {
                json::Dict dict;

                dict.emplace("type"s, "Bus"s);
                dict.emplace("bus"s, item.bus_name);
                dict.emplace("span_count"s, item.span_count);
                dict.emplace("time"s, item.time);

                array.push_back(std::move(dict));
            }
        }
        
        return json::Builder()
            .StartDict()
            .Key("request_id"s)
            .Value(stat.id)
            .Key("total_time"s)
            .Value(route_stat.total_time)
            .Key("items"s)
            .Value(std::move(array))
            .EndDict()
            .Build();
    } else {
        return Generate_Error_Message_Dict(stat.id);
    }
}

DBQueries ParseJson(const json::Document& document) {

    DBQueries result;

    const auto root_dict = document.GetRoot().AsDict();

    const auto& base_requests = root_dict.find("base_requests"s);
    const auto& render_settings = root_dict.find("render_settings"s);
    const auto& stat_requests = root_dict.find("stat_requests"s);
    const auto& routing_settings = root_dict.find("routing_settings"s);

    
    //read base_requests
    if (base_requests != root_dict.end()) {

        for (const auto& it : base_requests->second.AsArray()) {
            
            const auto& entry_dict = it.AsDict();
            const auto query_type_it = entry_dict.find("type"s);
            
            if (query_type_it != entry_dict.end()) {
                const auto& type_name = query_type_it->second.AsString();
                
                if (type_name == "Bus"s) {
                    Bus bus;
                    bus.name = entry_dict.at("name"s).AsString();
                    
                    for (auto stop : entry_dict.at("stops"s).AsArray()) {
                        bus.stops.push_back(std::move(stop.AsString()));
                    }
                    
                    bus.is_roundtrip = entry_dict.at("is_roundtrip"s).AsBool();
                    result.buses.push_back(std::move(bus));
                }
                
                if (type_name == "Stop"s) {
                    Stop stop;
                    stop.name = entry_dict.at("name"s).AsString();
                    stop.position.lat = entry_dict.at("latitude"s).AsDouble();
                    stop.position.lng = entry_dict.at("longitude"s).AsDouble();
                    
                    for (const auto& [stop_name, distance] : entry_dict.at("road_distances"s).AsDict()) {
                        stop.road_distances.emplace_back(stop_name, distance.AsInt());
                    }
                    
                    result.stops.push_back(std::move(stop));
                }
            }
        }
    }

    
    //read render_settings
    if (render_settings != root_dict.end()) {
        const auto render_map = render_settings->second.AsDict();

        result.render_settings.width = render_map.at("width"s).AsDouble();
        result.render_settings.height = render_map.at("height"s).AsDouble();
        result.render_settings.padding = render_map.at("padding"s).AsDouble();

        result.render_settings.line_width = render_map.at("line_width"s).AsDouble();
        result.render_settings.stop_radius = render_map.at("stop_radius"s).AsDouble();

        result.render_settings.bus_label_font_size = render_map.at("bus_label_font_size"s).AsInt();

        auto bus_label_offset_arr = render_map.at("bus_label_offset"s).AsArray();
        result.render_settings.bus_label_offset = {bus_label_offset_arr[0].AsDouble(), bus_label_offset_arr[1].AsDouble()};

        result.render_settings.stop_label_font_size = render_map.at("stop_label_font_size"s).AsInt();

        auto stop_label_offset_arr = render_map.at("stop_label_offset"s).AsArray();
        result.render_settings.stop_label_offset = {stop_label_offset_arr[0].AsDouble(), stop_label_offset_arr[1].AsDouble()};

        result.render_settings.underlayer_color = SetColor(render_map.at("underlayer_color"s));

        result.render_settings.underlayer_width = render_map.at("underlayer_width"s).AsDouble();

        auto color_palette = render_map.at("color_palette"s).AsArray();
        for (const auto& it : color_palette) {
            (result.render_settings.palette).push_back(SetColor(it));
        }

        result.render_settings.height = render_map.at("height"s).AsDouble();
    }

    
    //read stat_requests
    if (stat_requests != root_dict.end()) {

        for (const auto& it : stat_requests->second.AsArray()) {

            const auto& entry_dict = it.AsDict();

            Stat request;
            request.id = entry_dict.at("id").AsInt();
            std::string request_type = entry_dict.at("type").AsString();

            if (request_type == "Route"s) {
                request.type = RequestType::ROUTE;

                const auto from_it = entry_dict.find("from"s);
                const auto to_it = entry_dict.find("to"s);

                if (from_it != entry_dict.end() && to_it != entry_dict.end()) {
                    request.key_values["from"s] = from_it->second.AsString();
                    request.key_values["to"s] = to_it->second.AsString();
                }

                result.queries.push_back(std::move(request));
            }

            if (request_type == "Stop"s) {
                request.type = RequestType::STOP;
                const auto payload_it = entry_dict.find("name"s);
                if (payload_it != entry_dict.end()) {
                    request.key_values["name"s] = payload_it->second.AsString();
                }

                result.queries.push_back(std::move(request));
            }

            if (request_type == "Bus"s) {
                request.type = RequestType::BUS;
                const auto payload_it = entry_dict.find("name"s);
                if (payload_it != entry_dict.end()) {
                    request.key_values["name"s] = payload_it->second.AsString();
                }

                result.queries.push_back(std::move(request));
            }

            if (request_type == "Map"s) {
                request.type = RequestType::MAP;
                result.queries.push_back(std::move(request));
            }
        }
    }

    
    //read routing_settings
    if (routing_settings != root_dict.end()) {
        const auto routing_map = routing_settings->second.AsDict();

        result.routing_settings.bus_wait_time = routing_map.at("bus_wait_time"s).AsInt();

        int mkh = 1000;
        int minutes = 60;
        result.routing_settings.bus_velocity = mkh / double(minutes) * routing_map.at("bus_velocity"s).AsDouble();
    }

    return result;
}

svg::Color SetColor(const json::Node& color) {
        if (color.IsString()) {
            return color.AsString();
        } else {
            const auto& arr = color.AsArray();
            if (arr.size() == 3) {
                return svg::Rgb{static_cast<uint8_t>(arr[0].AsInt()),
                                static_cast<uint8_t>(arr[1].AsInt()),
                                static_cast<uint8_t>(arr [2].AsInt())};
            } else {
                return svg::Rgba{static_cast<uint8_t>(arr[0].AsInt()),
                                 static_cast<uint8_t>(arr[1].AsInt()),
                                 static_cast<uint8_t>(arr [2].AsInt()),
                                 arr[3].AsDouble()};
            }
        }
    }