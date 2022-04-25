#include "map_renderer.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

double MapRenderer::GetHeight() const {
    return rs_.height;
}
double MapRenderer::GetWidth() const {
    return rs_.width;
}

double MapRenderer::GetPadding() const {
    return rs_.padding;
}

void MapRenderer::AddBusRoutes(svg::Document& doc, const std::deque<svg::Point>& stops_points, size_t color_idx) const {
    svg::Polyline line;
    
    for (const auto& stop_point : stops_points) {
        line.AddPoint(stop_point)
            .SetStrokeColor(rs_.palette[color_idx])
            .SetStrokeWidth(rs_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetFillColor(svg::NoneColor)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }

    doc.Add(std::move(line));
}

void MapRenderer::AddBusNames(svg::Document& doc, const svg::Point& pos, const std::string& name, size_t color_idx) const {
    svg::Text text_underlayer;
    text_underlayer.SetFillColor(rs_.underlayer_color)
        .SetStrokeColor(rs_.underlayer_color)
        .SetStrokeWidth(rs_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
        .SetPosition(pos)
        .SetOffset(rs_.bus_label_offset)
        .SetFontSize(static_cast<uint32_t>(rs_.bus_label_font_size))
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(name);
    
    doc.Add(std::move(text_underlayer));
    
    svg::Text text_name;
    text_name.SetFillColor(rs_.palette[color_idx])
        .SetPosition(pos)
        .SetOffset(rs_.bus_label_offset)
        .SetFontSize(static_cast<uint32_t>(rs_.bus_label_font_size))
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetData(name);
    
    doc.Add(std::move(text_name));
}

void MapRenderer::AddStopPoints(svg::Document& doc, const Container_stops_points& points) const {

    for (const auto& point : points) {
        svg::Circle circle;
        circle.SetCenter(point.first)
            .SetRadius(rs_.stop_radius)
            .SetFillColor("white"s);
        
        doc.Add(circle);
    }
}

void MapRenderer::AddStopPoints(svg::Document& doc, const svg::Point& pos) const {
    AddStopPoints(doc, {pos});
}

void MapRenderer::AddStopNames(svg::Document& doc, const Container_stops_points& points) const {
    svg::Text text_common;

    text_common.SetOffset(rs_.stop_label_offset)
        .SetFontSize(static_cast<uint32_t>(rs_.stop_label_font_size))
        .SetFontFamily("Verdana");

    svg::Text bus_name_underlayer{ text_common };
    bus_name_underlayer.SetFillColor(rs_.underlayer_color)
        .SetStrokeColor(rs_.underlayer_color)
        .SetStrokeWidth(rs_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    svg::Text bus_name_title{ text_common };
    bus_name_title.SetFillColor("black"s);

    for (const auto& point : points) {
        bus_name_underlayer.SetData(std::string(point.second)).SetPosition(point.first);
        doc.Add(bus_name_underlayer);

        bus_name_title.SetData(std::string(point.second)).SetPosition(point.first);
        doc.Add(bus_name_title);
    }
}

void MapRenderer::AddStopNames(svg::Document& doc, const svg::Point& pos, std::string_view name) const {
    AddStopNames(doc, { pos }, name);
}

size_t MapRenderer::GetColor() const {
    return rs_.palette.size();
}
