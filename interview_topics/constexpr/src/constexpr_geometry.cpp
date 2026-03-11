// Shows constexpr-friendly geometry primitives and algorithms such as bounding
// boxes that can be evaluated during translation or reused at runtime.
#include <array>
#include <iostream>

struct Point {
    double x;
    double y;

    constexpr Point(double x_coord, double y_coord) : x{x_coord}, y{y_coord} {}

    constexpr Point translated(double dx, double dy) const {
        return Point{x + dx, y + dy};
    }
};

constexpr double squared_distance(Point a, Point b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return dx * dx + dy * dy;
}

constexpr auto bounding_box(std::array<Point, 4> points) {
    double min_x = points[0].x;
    double max_x = points[0].x;
    double min_y = points[0].y;
    double max_y = points[0].y;

    for (const auto& p : points) {
        if (p.x < min_x) min_x = p.x;
        if (p.x > max_x) max_x = p.x;
        if (p.y < min_y) min_y = p.y;
        if (p.y > max_y) max_y = p.y;
    }
    return std::array<double, 4>{min_x, min_y, max_x, max_y};
}

constexpr Point origin{0.0, 0.0};
constexpr Point unit_x{1.0, 0.0};

static_assert(squared_distance(origin, unit_x) == 1.0, "Unit distance failure");

int main() {
    constexpr std::array<Point, 4> quad = {
        Point{2.0, 3.5},
        Point{-1.0, 4.5},
        Point{3.0, -2.0},
        Point{0.5, 1.0},
    };

    constexpr auto bounds = bounding_box(quad);
    std::cout << "Bounding box (minX, minY, maxX, maxY): ";
    for (double value : bounds) {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    constexpr auto moved = unit_x.translated(-0.5, 2.0);
    std::cout << "Translated point: (" << moved.x << ", " << moved.y << ")\n";
}
