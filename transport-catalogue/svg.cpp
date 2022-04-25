#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        using namespace std::literals;
        if (line_cap == StrokeLineCap::BUTT) {
            out << "butt"sv;
        } else if (line_cap == StrokeLineCap::ROUND) {
            out << "round"sv;
        } else if (line_cap == StrokeLineCap::SQUARE) {
            out << "square"sv;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        using namespace std::literals;
        if (line_join == StrokeLineJoin::ARCS) {
            out << "arcs"sv;
        } else if (line_join == StrokeLineJoin::BEVEL) {
            out << "bevel"sv;
        } else if (line_join == StrokeLineJoin::MITER) {
            out << "miter"sv;
        } else if (line_join == StrokeLineJoin::MITER_CLIP) {
            out << "miter-clip"sv;
        } else if (line_join == StrokeLineJoin::ROUND) {
            out << "round"sv;
        }
        return out;
    }
    
    
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << "\n"sv;
    }

    // ---------- Circle ------------------
    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Poyline ------------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i) {
            if (i != 0)
                out << " "sv;
            out << points_[i].x << ","sv << points_[i].y;
        }
        out << "\" ";
        RenderAttrs(out);
        out << "/>";
    }

    // ---------- Text ------------------
    Text& Text::SetPosition(Point pos){
        pos_ = pos;
        return *this;
    }
    
    Text& Text::SetOffset(Point offset){
        offset_ = offset;
        return *this;
    }
    
    Text& Text::SetFontSize(uint32_t size){
        size_ = size;
        return *this;
    }
    
    Text& Text::SetFontFamily(std::string font_family){
        font_family_ = font_family;
        return *this;
    }
    
    Text& Text::SetFontWeight(std::string font_weight){
        font_weight_ = font_weight;
        return *this;
    }
    
    Text& Text::SetData(std::string data){
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text ";
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out<<">"sv << data_ << "</text>"sv;
    }

    // ---------- Document ------------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const{
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext context(out, 2, 2);
        for (auto &object_ : objects_){
            object_->Render(RenderContext{context});
        }
        out << "</svg>"sv;
    }

}