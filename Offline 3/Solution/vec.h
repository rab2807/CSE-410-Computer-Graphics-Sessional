#include <bits/stdc++.h>
using namespace std;

struct vect
{
    vect() {}
    double x, y, z;

    vect(double x, double y, double z) : x(x), y(y), z(z) {}
    vect(const vect &p) : x(p.x), y(p.y), z(p.z) {}

    double length() { return sqrt(x * x + y * y + z * z); }
    void normalize() { *this = *this / length(); }
    vect getNormalized() { return *this / length(); }
    double dot(vect b) { return x * b.x + y * b.y + z * b.z; }
    double distance(vect b) { return sqrt((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y) + (z - b.z) * (z - b.z)); }

    // arithemtic operations
    vect operator+(vect b) { return vect(x + b.x, y + b.y, z + b.z); }
    vect operator-(vect b) { return vect(x - b.x, y - b.y, z - b.z); }
    vect operator*(double b) { return vect(x * b, y * b, z * b); }
    vect operator/(double b) { return vect(x / b, y / b, z / b); }
    vect operator*(vect b) { return vect(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
    vect operator=(vect b)
    {
        x = b.x, y = b.y, z = b.z;
        return *this;
    }
    friend std::istream &operator>>(std::istream &in, vect &v)
    {
        in >> v.x >> v.y >> v.z;
        return in;
    }
    friend std::ostream &operator<<(std::ostream &out, vect &v)
    {
        out << v.x << " " << v.y << " " << v.z;
        return out;
    }
};