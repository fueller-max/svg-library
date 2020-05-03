#include<iostream>
#include<variant>
#include<optional>
#include<vector>
#include<string>
#include<utility>
#include<fstream>
#include<memory>


namespace  Svg {

    struct Point {
        double x = 0;
        double y = 0;
    };

    struct Rgb {
        uint8_t red{255};
        uint8_t green{255};
        uint8_t blue{255};
    };

    struct Color {
        std::variant<Rgb,std::string> _color;
        Color() :_color("none") {}
        Color(const std::string color) : _color(color) {}
        Color(const char* color) : _color(color) {}
        Color(const Rgb rgb) : _color(rgb){}


    };

    std::string DoubleToString(double num) {
        return  (num == static_cast<int>(num)) ? std::to_string(static_cast<int>(num)) :
                                                 std::to_string(num);
    }

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

    const Color NoneColor = Color();
    
    class SvgObject {
    public:
        virtual std::ostream& Render(std::ostream& os = std::cout) const = 0;
    };
    
    template<typename D>
	class SvgObjectTemplate : public SvgObject {
    public:
        D SetFillColor(const Color& colour = NoneColor) {
            fill_color = colour;
            return *static_cast<D*>(this);
        }
        D SetStrokeColor(const Color& colour = NoneColor) {
            stroke_color = colour;
            return *static_cast<D*>(this);
        }
        D SetStrokeWidth(double width = 1.0) {
            stroke_width = width;
            return *static_cast<D*>(this);
        }
        D SetStrokeLineCap(const std::string& str) {
            stroke_linecap = str;
            return *static_cast<D*>(this);
        }
        D SetStrokeLineJoin(const std::string& str) {
            stroke_linejoin = str;
            return *static_cast<D*>(this);
        }
        std::ostream& Render(std::ostream& os = std::cout) const {  
            os << "fill=\"" << ColorToString(fill_color) << "\" "
                << "stroke=\"" << ColorToString(stroke_color) << "\" "
                << "stroke-width=\"" << DoubleToString(stroke_width) << "\" ";
                stroke_linecap.has_value()  ? os << "stroke-linecap=\"" << stroke_linecap.value() << "\" " : os << "";
                stroke_linejoin.has_value() ? os << "stroke-linejoin=\"" << stroke_linejoin.value() << "\" " :os << "";
                return os;  
        };
    protected:
        Color fill_color = Svg::NoneColor;
        Color stroke_color = Svg::NoneColor; 
        double stroke_width = 1.0;
        std::optional<std::string> stroke_linecap;
        std::optional<std::string> stroke_linejoin;
    };
	

    class Circle : public SvgObjectTemplate<Circle> {
    public:
        Circle SetCenter(Point center = {0.0,0.0}) {
            _center = std::move(center);
            return *this;
        }
        Circle SetRadius(double radius = 1.0) {
            _radius = radius;
            return *this;
        }
        std::ostream& Render(std::ostream& os = std::cout) const {
            os << "<circle " << "cx=\"" << DoubleToString(_center.x) << "\" "
                << "cy=\"" << DoubleToString(_center.y) << "\" "
                << "r=\"" << DoubleToString(_radius) << "\" ";
                SvgObjectTemplate::Render(os)
                << "/>";
            return os;
        };
    
    private:
        Point _center;
        double _radius = 1.0;
    };
	


    class Polyline : public SvgObjectTemplate<Polyline> {
    public:
        Polyline() = default;
        Polyline AddPoint(Point point) {
            vertexes.push_back(std::move(point));
            return *this;
        }
        std::ostream& Render(std::ostream& os = std::cout) const {
            os << "<polyline " << "points=\"" << GetVertexes() << "\" ";
               SvgObjectTemplate::Render(os)
               <<"/>";
            return os;
        };
       
    private:
        std::string GetVertexes() const {
            std::string points;
            for (const auto [x, y] : vertexes) {
                points += (DoubleToString(x) + ", " + DoubleToString(y) + " ");
            }
            return points;
        }
        
        std::vector<Point> vertexes = {};
    };



    class Text : public SvgObjectTemplate<Text> {
    public:
        Text SetPoint(Point point = {}) {
            reference_point = std::move(point);
            return *this;
        }
        Text SetOffset(Point offset = {}) {
            _offset = std::move(offset);
            return *this;
        }
        Text SetFontSize(uint32_t size = 1) {
            font_size = size;
            return *this;
        }
        Text SetFontFamily(const std::string& font) {
            _font_family = font;
            return *this;
        }
        Text SetData(const std::string& data) {
            text = data;
            return *this;
        }
        std::ostream& Render(std::ostream& os = std::cout) const {
            os << "<text " << "x=\"" << DoubleToString(reference_point.x) << "\" "
                << "y=\"" << DoubleToString(reference_point.y) << "\" "
                << "dx=\"" << DoubleToString(_offset.x) << "\" "
                << "dy=\"" << DoubleToString(_offset.y) << "\" "
                << "font-size=\"" << font_size << "\" ";
                _font_family.has_value() ? os << "font-family=\"" << _font_family.value() << "\" " : os<<"";
                SvgObjectTemplate::Render(os)
                    << ">" << text << "</text>";
               // << "/>";
            return os;
        };
    private:
        std::string text ="";
        Point  reference_point;
        Point _offset ;
        uint32_t font_size = 1;
        std::optional<std::string> _font_family;
    };




	class Document {
		public:
		Document() = default;

        template<typename SvgObj>
		void Add(SvgObj&& svg_obj) {
            std::unique_ptr<SvgObj> ptr_obj(std::make_unique<SvgObj>(svg_obj));
            svg_objects.push_back(std::move(ptr_obj));
        }

		void Render(std::ostream& os = std::cout) const {
            os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << '\n'
               << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" <<'\n';
         
            for (size_t idx = 0; idx < svg_objects.size(); ++idx) {    // Rendering of all SvgOjects...
            
                svg_objects[idx] -> Render(os);
            }
    
            os << "</svg>";
        }
    
    private:
   
        std::vector<std::unique_ptr<SvgObject>> svg_objects;
    };

}

void Test() {
    Svg::Document svg;
  
    svg.Add(
        Svg::Polyline{}
        .SetStrokeColor(Svg::Rgb{ 140, 198, 63 })  // soft green
        .SetStrokeWidth(16)
        .SetStrokeLineCap("round")
        .AddPoint({ 50, 50 })
        .AddPoint({ 250, 250 })
    );

    for (const auto point : { Svg::Point{50, 50}, Svg::Point{250, 250} }) {
        svg.Add(
            Svg::Circle{}
            .SetFillColor("white")
            .SetRadius(6)
            .SetCenter(point)
        );
    }

    svg.Add(
        Svg::Text{}
        .SetPoint({ 50, 50 })
        .SetOffset({ 10, -10 })
        .SetFontSize(20)
        .SetFontFamily("Verdana")
        .SetFillColor("black")
        .SetData("C")
    );
    svg.Add(
        Svg::Text{}
        .SetPoint({ 250, 250 })
        .SetOffset({ 10, -10 })
        .SetFontSize(50)
        .SetFontFamily("Calibri")
        .SetFillColor("black")
        .SetData("C++")
    );

    std::ofstream write_to_file("test.svg");
    svg.Render(write_to_file);
    svg.Render(std::cout);

}

void Test1() {
    Svg::Document svg;

    svg.Add(
        Svg::Polyline{}
        .SetStrokeColor(Svg::Rgb{ 140, 198, 63 })  // soft green
        .SetStrokeWidth(16)
        .SetStrokeLineCap("round")
        .AddPoint({ 50.34, 50.67 })
        .AddPoint({ 200, 100 })
        .AddPoint({ 250, 250 })
    );

    for (const auto point : { Svg::Point{50, 50}, Svg::Point{250, 250} }) {
        svg.Add(
            Svg::Circle{}
            .SetFillColor("white")
            .SetRadius(6)
            .SetCenter(point)
        );
    }

    svg.Add(
        Svg::Text{}
        .SetPoint({ 50, 50 })
        .SetOffset({ 10, -10 })
        .SetFontSize(20)
        .SetFontFamily("Verdana")
        .SetFillColor("black")
        .SetData("C")
    );
    svg.Add(
        Svg::Text{}
       // .SetPoint({ 250, 250 })
      //  .SetOffset({ 10, -10 })
      //  .SetFontSize(50)
       // .SetFontFamily("Calibri")
        .SetStrokeColor("my color")
        .SetFillColor("black")
        .SetData("C++")
    );

    std::ofstream write_to_file("test1.svg");
   // svg.Render(write_to_file);
      svg.Render(std::cout);

}


void Test2() {
    Svg::Document svg;

   /* svg.Add(
        Svg::Polyline{}
        .SetStrokeColor(Svg::Rgb{ 50, 50, 63 })  
        .SetStrokeWidth(50)
        .SetStrokeLineCap("round")
        .AddPoint({ 500.34, 500.67 })
        .AddPoint({ 200.00006, 100.435353 })
        .AddPoint({ 250.543545, 250.587686 })
    );

    for (const auto point : { Svg::Point{50, 50}, Svg::Point{250, 250} }) {
        svg.Add(
            Svg::Circle{}
            .SetFillColor("white")
            .SetRadius(6)
            .SetCenter(point)
        );
    }*/

   /* svg.Add(
        Svg::Text{}
        .SetPoint({ 50, 50 })
        .SetOffset({ 10, -10 })
        .SetFontSize(20)
        .SetFontFamily("Verdana")
        .SetFillColor("black")
       // .SetData("")
    );*/
    svg.Add(
        Svg::Text{}
        // .SetPoint({ 250, 250 })
       //  .SetOffset({ 10, -10 })
       //  .SetFontSize(50)
        // .SetFontFamily("Calibri")
        .SetStrokeColor("my color")
        .SetFillColor("black")
        .SetData("C++")
    );

    std::ofstream write_to_file("test1.svg");
     svg.Render(write_to_file);
    svg.Render(std::cout);

}

int main() {
    Test();
	return 0;
}