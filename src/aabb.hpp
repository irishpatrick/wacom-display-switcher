#ifndef AABB_HPP
#define AABB_HPP

class AABB {
public:
    explicit AABB(double x = 0, double y = 0, double w = 0, double h = 0)
            : x(x), y(y), w(w), h(h) {
    }

    [[nodiscard]] bool intersects(double px, double py) const {
        return px > x && px < x + w && py > y && py < y + h;
    }

    double x;
    double y;
    double w;
    double h;
};

#endif /* AABB_HPP */