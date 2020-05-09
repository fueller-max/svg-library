#pragma once

#include<iostream>
#include<variant>
#include<optional>
#include<vector>
#include<string>
#include<utility>
#include<memory>



namespace  Svg {

    struct Point {
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        uint8_t red{ 255 };
        uint8_t green{ 255 };
        uint8_t blue{ 255 };
    };

    struct Color {
        std::variant<Rgb, std::string> _color;
        Color() :_color("none") {}
        Color(const std::string color) : _color(color) {}
        Color(const char* color) : _color(color) {}
        Color(const Rgb rgb) : _color(rgb) {}


    };

    const Color NoneColor = Color();

    std::string ColorToString(const Color& color);


    class SvgObject {
    public:
        virtual std::ostream& Render(std::ostream& os = std::cout) const = 0;
        virtual ~SvgObject() = default;
    };

    template<typename D>
    class SvgObjectCoomonProp {
    public:
        D SetFillColor(const Color& colour = NoneColor);
        D SetStrokeColor(const Color& colour = NoneColor);
        D SetStrokeWidth(double width = 1.0);
        D SetStrokeLineCap(const std::string& str);
        D SetStrokeLineJoin(const std::string& str);

        std::ostream& Render(std::ostream& os = std::cout) const;
        virtual ~SvgObjectCoomonProp() = default;
    protected:
        Color fill_color = Svg::NoneColor;
        Color stroke_color = Svg::NoneColor;
        double stroke_width = 1.0;
        std::optional<std::string> stroke_linecap;
        std::optional<std::string> stroke_linejoin;
    };

    class Circle : public SvgObject, public SvgObjectCoomonProp<Circle> {
    public:
        Circle SetCenter(Point center = { 0,0 });
        Circle SetRadius(double radius = 1.0);
        std::ostream& Render(std::ostream& os = std::cout) const;

    private:
        Point _center;
        double _radius = 1.0;
    };


    class Polyline : public SvgObject, public SvgObjectCoomonProp<Polyline> {
    public:
        Polyline() = default;
        Polyline AddPoint(Point point);
        std::ostream& Render(std::ostream& os = std::cout) const;

    private:
        std::ostream& GetVertexes(std::ostream& os = std::cout) const;
        std::vector<Point> vertexes = {};
    };

    class Text : public SvgObject, public SvgObjectCoomonProp<Text> {
    public:
        Text SetPoint(Point point = {}); 
        Text SetOffset(Point offset = {}); 
        Text SetFontSize(uint32_t size = 1); 
        Text SetFontFamily(const std::string& font); 
        Text SetData(const std::string& data); 
        std::ostream& Render(std::ostream& os = std::cout) const; 
    private:
        std::string text = "";
        Point  reference_point;
        Point _offset;
        uint32_t font_size = 1;
        std::optional<std::string> _font_family;
    };

    class Document {
    public:
        Document() = default;

        template<typename SvgObj>
        void Add(SvgObj&& svg_obj);
        void Render(std::ostream& os = std::cout) const; 

     
    private:
        std::vector<std::unique_ptr<SvgObject>> svg_objects;
    };


    std::string ColorToString(const Color& color) {
        if (std::holds_alternative<std::string>(color._color))
        {
            return std::get<std::string>(color._color);
        }
        else {
            return "rgb(" + std::to_string(std::get<Svg::Rgb>(color._color).red) + "," +
                std::to_string(std::get<Svg::Rgb>(color._color).green) + "," +
                std::to_string(std::get<Svg::Rgb>(color._color).blue) + ")";
        }

    }




    template<typename D>
    D SvgObjectCoomonProp<D>::SetFillColor(const Color& colour) {
        fill_color = colour;
        return *static_cast<D*>(this);
    }

    template<typename D>
    D SvgObjectCoomonProp<D>::SetStrokeColor(const Color& colour) {
        stroke_color = colour;
        return *static_cast<D*>(this);
    }

    template<typename D>
    D SvgObjectCoomonProp<D>::SetStrokeWidth(double width) {
        stroke_width = width;
        return *static_cast<D*>(this);
    }
    template<typename D>
    D SvgObjectCoomonProp<D>::SetStrokeLineCap(const std::string& str) {
        stroke_linecap = str;
        return *static_cast<D*>(this);
    }
    template<typename D>
    D SvgObjectCoomonProp<D>::SetStrokeLineJoin(const std::string& str) {
        stroke_linejoin = str;
        return *static_cast<D*>(this);
    }
    template<typename D>

    std::ostream& SvgObjectCoomonProp<D>::Render(std::ostream& os) const {
        os << "fill=\"" << ColorToString(fill_color) << "\" "
            << "stroke=\"" << ColorToString(stroke_color) << "\" "
            << "stroke-width=\"" << stroke_width << "\" ";
        if (stroke_linecap.has_value())  os << "stroke-linecap=\"" << stroke_linecap.value() << "\" ";
        if (stroke_linejoin.has_value()) os << "stroke-linejoin=\"" << stroke_linejoin.value() << "\" ";
        return os;
    };





    Circle Circle::SetCenter(Point center) {
        _center = std::move(center);
        return *this;
    }
    Circle Circle::SetRadius(double radius) {
        _radius = radius;
        return *this;
    }
    std::ostream& Circle::Render(std::ostream& os) const {
        os << "<circle " << "cx=\"" << (_center.x) << "\" "
            << "cy=\"" << (_center.y) << "\" "
            << "r=\"" << (_radius) << "\" ";
        SvgObjectCoomonProp::Render(os)
            << "/>";
        return os;
    };





    Polyline Polyline::AddPoint(Point point) {
        vertexes.push_back(std::move(point));
        return *this;
    }
    std::ostream& Polyline::Render(std::ostream& os) const {
        os << "<polyline " << "points=\"";
        GetVertexes(os) << "\" ";
        SvgObjectCoomonProp::Render(os)
            << "/>";
        return os;
    };

    std::ostream& Polyline::GetVertexes(std::ostream& os) const {
        for (const auto [x, y] : vertexes) {
            os << x << "," << y << " ";
        }
        return os;
    }




    Text Text::SetPoint(Point point) {
        reference_point = std::move(point);
        return *this;
    }
    Text Text::SetOffset(Point offset) {
        _offset = std::move(offset);
        return *this;
    }
    Text Text::SetFontSize(uint32_t size) {
        font_size = size;
        return *this;
    }
    Text Text::SetFontFamily(const std::string& font) {
        _font_family = font;
        return *this;
    }
    Text Text::SetData(const std::string& data) {
        text = data;
        return *this;
    }
    std::ostream& Text::Render(std::ostream& os) const {
        os << "<text " << "x=\"" << reference_point.x << "\" "
            << "y=\"" << reference_point.y << "\" "
            << "dx=\"" << _offset.x << "\" "
            << "dy=\"" << _offset.y << "\" "
            << "font-size=\"" << font_size << "\" ";
        if (_font_family.has_value()) os << "font-family=\"" << _font_family.value() << "\" ";
        SvgObjectCoomonProp::Render(os)
            << ">" << text << "</text>";
        // << "/>";
        return os;
    };



    template<typename SvgObj>
    void Document::Add(SvgObj&& svg_obj) {
        std::unique_ptr<SvgObj> ptr_obj(std::make_unique<SvgObj>(svg_obj));
        svg_objects.push_back(std::move(ptr_obj));
    }

    void Document::Render(std::ostream& os) const {
        os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << '\n'
            << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << '\n';

        for (size_t idx = 0; idx < svg_objects.size(); ++idx) {    // Rendering of all SvgOjects...

            svg_objects[idx]->Render(os);
        }

        os << "</svg>";
    }
}
