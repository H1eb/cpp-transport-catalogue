#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

using namespace std::literals;

namespace svg {
    
    struct Rgb{
        Rgb() = default;
        
        Rgb(uint8_t r, uint8_t g, uint8_t b) 
            : red(r)
            , green(g)
            , blue(b){
            }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    
    std::ostream& operator<<(std::ostream& out, const Rgb& rgb);
    
    struct Rgba {
        Rgba() = default;
        
        Rgba(uint8_t r, uint8_t g, uint8_t b, double a) 
            : red(r)
            , green(g)
            , blue(b)
            , opacity(a){
            }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{};
    
    struct ColorPrinter {
		std::ostream& out;
		void operator()(std::monostate) const {
			using namespace std::literals;
			out << "none"sv;
		}
		void operator()(const std::string_view str) const {
			using namespace std::literals;
			out << str;
		}
		void operator()(const Rgb& rgb) {
			using namespace std::literals;
			out << "rgb("sv << static_cast<int>(rgb.red)
                << ',' << static_cast<int>(rgb.green)
                << ',' << static_cast<int>(rgb.blue) << ')';
		}
		void operator()(const Rgba& rgba) {
			using namespace std::literals;
			out << "rgba("sv << static_cast<int>(rgba.red)
                << ',' << static_cast<int>(rgba.green)
                << ',' << static_cast<int>(rgba.blue)
                << ',' << rgba.opacity << ')';
		}
	};

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& os, svg::StrokeLineCap slc);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& os, StrokeLineJoin slj);

    template <typename Owner>
    class PathProps{
    public:
        Owner& SetFillColor(Color color){
            fill_color_ = std::move(color);
            return AsOwner();
        }
        
        Owner& SetStrokeColor(Color color){
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        
        Owner& SetStrokeWidth(double width){
            line_width_ = width;
            return AsOwner();
        }
        
        Owner& SetStrokeLineCap(StrokeLineCap line_cap){
            line_cap_ = line_cap;
            return AsOwner();
        }
        
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join){
            line_join_ = line_join;
            return AsOwner();
        }
        
    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;

            if (fill_color_) {
                out << "fill=\""sv;
                std::visit(ColorPrinter{out}, *fill_color_);
                out << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(ColorPrinter{out}, *stroke_color_);
                out << "\""sv;
            }
            if (line_width_){
                out << " stroke-width=\""sv << *line_width_ << "\" "sv;
            }
            
            if (line_cap_){
                out << "stroke-linecap=\""sv << *line_cap_ << "\" "sv;
            }
            
            if (line_join_){
                out << "stroke-linejoin=\""sv << *line_join_ << "\" "sv;
            }
        }
        
    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> line_width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };
    
    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }

        double x = 0;
        double y = 0;
    };

    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };
    
    

    class Circle : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline>{
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        std::vector<Point> points_;
    };

    class Text : public Object, public PathProps<Text>{
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_;
        Point offset_;
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class ObjectContainer {
    public:
        template <typename Object>
        void Add(Object obj) {
            objects_.emplace_back(std::make_unique<Object>(std::move(obj)));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() = default;

    protected:
        std::deque<std::shared_ptr<Object>> objects_;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& oc) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer{
    public:
        void AddPtr(std::unique_ptr<Object>&& obj);
        void Render(std::ostream& out) const;
    };

}