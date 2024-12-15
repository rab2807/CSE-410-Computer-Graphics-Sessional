#include <bits/stdc++.h>
#include "mat.h"
using namespace std;

class vect
{
public:
    double x, y, z, w;

    vect()
    {
        x = 0;
        y = 0;
        z = 0;
        w = 1;
    }

    vect(double x, double y, double z, double w = 1)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    matrix toMatrix()
    {
        matrix result(4, 1);

        result.mat[0][0] = x;
        result.mat[1][0] = y;
        result.mat[2][0] = z;
        result.mat[3][0] = w;

        return result;
    }

    vect toVect(matrix m)
    {
        if (m.row != 4 || m.col != 1)
        {
            cout << "Error: Matrix dimensions do not match" << endl;
            return vect();
        }

        vect result(m.mat[0][0], m.mat[1][0], m.mat[2][0], m.mat[3][0]);
        return result;
    }

    vect normalize()
    {
        double len = sqrt(x * x + y * y + z * z);
        x /= len;
        y /= len;
        z /= len;
        return *this;
    }

    vect homogeneous()
    {
        if (w == 1)
            return *this;
        x /= w;
        y /= w;
        z /= w;
        w /= w;
        return *this;
    }

    vect cross(const vect &v)
    {
        double x_cross = y * v.z - z * v.y;
        double y_cross = z * v.x - x * v.z;
        double z_cross = x * v.y - y * v.x;
        return vect(x_cross, y_cross, z_cross);
    }

    double dot(const vect &v)
    {
        return x * v.x + y * v.y + z * v.z;
    }

    vect operator+(const vect &v) const
    {
        return vect(x + v.x, y + v.y, z + v.z);
    }

    vect operator-(const vect &v) const
    {
        return vect(x - v.x, y - v.y, z - v.z);
    }

    vect operator*(double s) const
    {
        return vect(s * x, s * y, s * z);
    }

    vect operator/(double s) const
    {
        return vect(x / s, y / s, z / s);
    }

    vect operator=(const vect &v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;
        return *this;
    }

    friend ostream &operator<<(ostream &os, const vect &v)
    {
        os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }
};
