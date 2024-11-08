#ifndef AABB_HPP
#define AABB_HPP

class AABB
{
public:
    AABB(double x = 0, double y = 0, double w = 0, double h = 0)
        : x(x), y(y), w(w), h(h)
    {
    }

    bool intersects(double x, double y) const
    {
        return x > this->x && x < this->x + this->w && y > this->y && y < this->y + this->h;
    }

    double x;
    double y;
    double w;
    double h;
};

#endif /* AABB_HPP */