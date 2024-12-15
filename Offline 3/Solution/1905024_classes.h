#include <bits/stdc++.h>
#include "bitmap_image.hpp"
#include "vec.h"
#include <GL/glut.h>
using namespace std;

#define pi (2 * acos(0.0))
#define EPS 1e-5

extern bitmap_image image;

struct Color
{
    double r, g, b;
    Color()
    {
        r = g = b = 0.0;
    }

    Color(double r, double g, double b) : r(r), g(g), b(b) {}

    Color operator+(const Color &c)
    {
        return Color(r + c.r, g + c.g, b + c.b);
    }

    Color operator*(const Color &c)
    {
        return Color(r * c.r, g * c.g, b * c.b);
    }

    Color operator*(const double &c)
    {
        return Color(r * c, g * c, b * c);
    }

    Color operator/(const double &c)
    {
        return Color(r / c, g / c, b / c);
    }

    void regulate()
    {
        r = min(1.0, r);
        g = min(1.0, g);
        b = min(1.0, b);
    }
};

struct Ray
{
    vect origin, dir;
    bool isInside;
    double currentRefractiveIndex;

    Ray(vect origin, vect dir)
    {
        this->origin = origin;
        this->dir = dir.getNormalized();
        isInside = false;
        currentRefractiveIndex = 1.0;
    }

    friend ostream &operator<<(ostream &out, Ray r)
    {
        out << "Origin : " << r.origin << ", Direction : " << r.dir;
        return out;
    }
};

struct Light
{
    vect pos;
    Color color;

    void draw()
    {
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glColor3f(color.r, color.g, color.b);
        glutSolidSphere(1, 10, 10);
        glPopMatrix();
    }

    friend istream &operator>>(istream &in, Light &l)
    {
        in >> l.pos.x >> l.pos.y >> l.pos.z;
        in >> l.color.r >> l.color.g >> l.color.b;
        return in;
    }
};

struct SpotLight : public Light
{
    vect dir;
    double cutoffAngle;

    bool exceedsCutoff(vect intersectionPoint)
    {
        vect lightDirection = intersectionPoint - pos;
        lightDirection.normalize();
        double dot = lightDirection.dot(dir);
        dot /= lightDirection.length() * dir.length();
        double theta = acos(abs(dot));

        return (theta > (cutoffAngle * pi / 180.0));
    }

    void draw()
    {
        glPushMatrix();
        glTranslatef(pos.x, pos.y, pos.z);
        glColor3f(color.r, color.g, color.b);
        glutSolidSphere(1, 10, 10);
        glPopMatrix();
    }

    friend istream &operator>>(istream &in, SpotLight &l)
    {
        in >> l.pos;
        in >> l.color.r >> l.color.g >> l.color.b;
        in >> l.dir;
        in >> l.cutoffAngle;
        return in;
    }
};

class Object;

extern vector<Light *> lights;
extern vector<SpotLight *> spotlights;
extern vector<Object *> objects;
extern int recursionLevel;

class Object
{
public:
    vect reference_point;
    double height, width, length;
    Color color;
    vector<double> coefficients;
    double refractiveIndex;
    int shine;

    Object()
    {
        coefficients = vector<double>(5, 0);
    }

    void setColor(Color color)
    {
        this->color = color;
    }

    virtual Color getColorAt(vect point)
    {
        return Color(this->color.r, this->color.g, this->color.b);
    }

    void setShine(int shine)
    {
        this->shine = shine;
    }

    void setCoefficients(double coefficients[])
    {
        for (int i = 0; i < 4; i++)
            this->coefficients[i] = coefficients[i];
    }

    virtual void draw() = 0;
    virtual double getT(Ray ray, Color &color, int level) = 0;
    virtual Ray getNormal(vect point, Ray incidentRay) = 0;

    Color getPhongColor(Ray ray, vect &point, Color &color)
    {

        Color finalColor = color * coefficients[0];

        for (Light *light : lights)
        {
            vect lightPosition = light->pos;
            vect lightDirection = point - lightPosition;

            Ray lightRay = Ray(lightPosition, lightDirection.getNormalized());

            double lightRayDistance = lightDirection.length();
            double mini = lightRayDistance, t;

            for (Object *obj : objects)
            {
                t = obj->getT(lightRay, color, 0);
                if (t > 0 && t < mini)
                    mini = t;
            }

            if (lightRayDistance < 0 || mini < lightRayDistance)
                continue;

            Ray normal = getNormal(point, lightRay);

            double diffuseFactor = max(0.0, -lightRay.dir.dot(normal.dir));
            finalColor = finalColor + light->color * coefficients[1] * diffuseFactor * color;
            finalColor.regulate();

            Ray reflection = Ray(point, lightRay.dir - normal.dir * 2 * (lightRay.dir.dot(normal.dir)));
            double phong = max(0.0, -ray.dir.dot(reflection.dir));
            finalColor = finalColor + light->color * coefficients[2] * pow(phong, shine) * color;
            finalColor.regulate();
        }

        for (SpotLight *spotlight : spotlights)
        {
            vect lightPosition = spotlight->pos;
            vect lightDirection = point - lightPosition;
            lightDirection.normalize();

            Ray lightRay = Ray(lightPosition, lightDirection);
            double dot = lightDirection.dot(spotlight->dir);
            double angle = acos(dot / (lightDirection.length() * spotlight->dir.length())) * (180.0 / pi);

            if (fabs(angle) > spotlight->cutoffAngle)
                continue;

            double lightRayDistance = (point - lightPosition).length();
            double mini = lightRayDistance, t;

            for (Object *obj : objects)
            {
                t = obj->getT(lightRay, color, 0);
                if (t > 0 && t < mini)
                    mini = t;
            }

            if (lightRayDistance < EPS || mini + EPS < lightRayDistance)
                continue;

            Ray normal = getNormal(point, lightRay);

            double diffuseFactor = max(0.0, -lightRay.dir.dot(normal.dir));
            finalColor = finalColor + spotlight->color * coefficients[1] * diffuseFactor * color;
            finalColor.regulate();

            Ray reflection = Ray(point, lightRay.dir - normal.dir * 2 * (lightRay.dir.dot(normal.dir)));
            double phong = max(0.0, -ray.dir.dot(reflection.dir));
            finalColor = finalColor + spotlight->color * coefficients[2] * pow(phong, shine) * color;
            finalColor.regulate();
        }

        return finalColor;
    }

    int getNearestObject(Ray ray)
    {
        double t = -1, tMin = 1e9;
        int nearestObjectIndex = -1;
        for (int k = 0; k < objects.size(); k++)
        {
            Color color(0, 0, 0);
            t = objects[k]->intersect(ray, color, 0);
            if (t > 0 && t < tMin)
            {
                tMin = t;
                nearestObjectIndex = k;
            }
        }

        return nearestObjectIndex;
    }

    virtual double intersect(Ray ray, Color &color, int level)
    {
        double t = getT(ray, color, level);

        if (t < 0)
            return -1;
        if (level == 0)
            return t;

        vect intersectionPoint = ray.origin + ray.dir * t;
        Color colorAtIntersection = getColorAt(intersectionPoint);

        color = getPhongColor(ray, intersectionPoint, colorAtIntersection);

        if (level < recursionLevel)
        {
            Ray normal = getNormal(intersectionPoint, ray);

            Ray reflectedRay = Ray(intersectionPoint, ray.dir - normal.dir * 2 * (ray.dir.dot(normal.dir)));
            reflectedRay.origin = reflectedRay.origin + reflectedRay.dir * EPS;

            nearestObjectIndex = getNearestObject(reflectedRay);

            if (nearestObjectIndex != -1)
            {
                Color reflectedColor(0, 0, 0);

                double t = objects[nearestObjectIndex]->intersect(reflectedRay, reflectedColor, level + 1);
                color = color + reflectedColor * coefficients[3];
                color.regulate();
            }
        }

        return t;
    }

    virtual ~Object()
    {
        coefficients.clear();
    }
};

struct General : public Object
{
    double A, B, C, D, E, F, G, H, I, J;

    General()
    {
    }

    virtual void draw()
    {
        return;
    }

    virtual Ray getNormal(vect intersectionPoint, Ray incidentRay)
    {
        vect normal(2 * A * intersectionPoint.x + D * intersectionPoint.y + E * intersectionPoint.z + G,
                    2 * B * intersectionPoint.y + D * intersectionPoint.x + F * intersectionPoint.z + H,
                    2 * C * intersectionPoint.z + E * intersectionPoint.x + F * intersectionPoint.y + I);

        normal.normalize();

        if (incidentRay.dir.dot(normal) > 0)
            normal = normal * -1;

        return Ray(intersectionPoint, normal);
    }

    bool isValid(vect point)
    {
        if (abs(length) > EPS && (point.x < reference_point.x || point.x > reference_point.x + length))
            return false;

        if (abs(width) > EPS && (point.y < reference_point.y || point.y > reference_point.y + width))
            return false;

        if (abs(height) > EPS && (point.z < reference_point.z || point.z > reference_point.z + height))
            return false;

        return true;
    }

    virtual double getT(Ray ray, Color &color, int level)
    {
        double X0 = ray.origin.x;
        double Y0 = ray.origin.y;
        double Z0 = ray.origin.z;

        double X1 = ray.dir.x;
        double Y1 = ray.dir.y;
        double Z1 = ray.dir.z;

        double a = A * X1 * X1 + B * Y1 * Y1 + C * Z1 * Z1;
        a += D * X1 * Y1 + E * X1 * Z1 + F * Y1 * Z1;

        double b = 2 * A * X0 * X1 + 2 * B * Y0 * Y1 + 2 * C * Z0 * Z1;
        b += D * (X0 * Y1 + X1 * Y0);
        b += E * (X0 * Z1 + X1 * Z0);
        b += F * (Y0 * Z1 + Y1 * Z0);
        b += G * X1 + H * Y1 + I * Z1;

        double c = A * X0 * X0 + B * Y0 * Y0 + C * Z0 * Z0;
        c += D * X0 * Y0 + E * X0 * Z0 + F * Y0 * Z0;
        c += G * X0 + H * Y0 + I * Z0 + J;

        double discriminant = b * b - 4 * a * c;
        if (discriminant < 0)
            return -1;
        if (a == 0)
        {
            double t = -c / b;
            vect intersectionPoint = ray.origin + ray.dir * t;
            if (!isValid(intersectionPoint))
                return -1;
            return t;
        }

        double t1 = (-b - sqrt(discriminant)) / (2 * a);
        double t2 = (-b + sqrt(discriminant)) / (2 * a);

        vect int1 = ray.origin + ray.dir * t1;
        vect int2 = ray.origin + ray.dir * t2;
        double t = -1;

        if (t1 > 0 && isValid(int1))
            t = t1;
        else if (t2 > 0 && isValid(int2))
            t = t2;

        return t;
    }

    friend istream &operator>>(istream &in, General &g)
    {
        in >> g.A >> g.B >> g.C >> g.D >> g.E >> g.F >> g.G >> g.H >> g.I >> g.J;
        in >> g.reference_point >> g.length >> g.width >> g.height;

        in >> g.color.r >> g.color.g >> g.color.b;
        for (int i = 0; i < 4; i++)
            in >> g.coefficients[i];
        in >> g.shine;
        return in;
    }
};

struct Triangle : public Object
{
private:
    double det(double ara[3][3])
    {
        double v1 = ara[0][0] * (ara[1][1] * ara[2][2] - ara[1][2] * ara[2][1]);
        double v2 = ara[0][1] * (ara[1][0] * ara[2][2] - ara[1][2] * ara[2][0]);
        double v3 = ara[0][2] * (ara[1][0] * ara[2][1] - ara[1][1] * ara[2][0]);
        return v1 - v2 + v3;
    }

public:
    vect a, b, c;

    Triangle()
    {
    }

    Triangle(vect a, vect b, vect c)
    {
        this->a = a;
        this->b = b;
        this->c = c;
    }

    virtual Ray getNormal(vect point, Ray incidentRay)
    {
        vect normal = (b - a) * (c - a);
        normal.normalize();
        if ((incidentRay.origin - this->reference_point).dot(normal) > 0)
            normal = normal * -1;
        return Ray(point, normal);
    }

    virtual void draw()
    {
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_TRIANGLES);
        {
            glVertex3f(a.x, a.y, a.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(c.x, c.y, c.z);
        }
        glEnd();
    }

    virtual double getT(Ray ray, Color &color, int level)
    {

        double betaMat[3][3] = {
            {a.x - ray.origin.x, a.x - c.x, ray.dir.x},
            {a.y - ray.origin.y, a.y - c.y, ray.dir.y},
            {a.z - ray.origin.z, a.z - c.z, ray.dir.z}};
        double gammaMat[3][3] = {
            {a.x - b.x, a.x - ray.origin.x, ray.dir.x},
            {a.y - b.y, a.y - ray.origin.y, ray.dir.y},
            {a.z - b.z, a.z - ray.origin.z, ray.dir.z}};
        double tMat[3][3] = {
            {a.x - b.x, a.x - c.x, a.x - ray.origin.x},
            {a.y - b.y, a.y - c.y, a.y - ray.origin.y},
            {a.z - b.z, a.z - c.z, a.z - ray.origin.z}};
        double AMat[3][3]{
            {a.x - b.x, a.x - c.x, ray.dir.x},
            {a.y - b.y, a.y - c.y, ray.dir.y},
            {a.z - b.z, a.z - c.z, ray.dir.z}};

        double Adet = det(AMat);
        double beta = det(betaMat) / Adet;
        double gamma = det(gammaMat) / Adet;
        double t = det(tMat) / Adet;

        if (beta + gamma < 1 && beta > 0 && gamma > 0 && t > 0)
            return t;
        return -1;
    }

    friend istream &operator>>(istream &in, Triangle &t)
    {
        in >> t.a >> t.b >> t.c;
        in >> t.color.r >> t.color.g >> t.color.b;
        for (int i = 0; i < 4; i++)
            in >> t.coefficients[i];
        in >> t.shine;
        return in;
    }
};

struct Sphere : public Object
{

    Sphere()
    {
    }

    Sphere(vect center, double radius)
    {
        reference_point = center;
        length = radius;
    }

    virtual Ray getNormal(vect point, Ray incidentRay)
    {
        Ray normal(point, point - reference_point);

        if (normal.dir.dot(incidentRay.dir) > 0)
            normal.dir = normal.dir * -1;
        return normal;
    }

    virtual void draw()
    {
        int stacks = 30;
        int slices = 20;

        vect points[100][100];
        int i, j;
        double h, r;

        for (i = 0; i <= stacks; i++)
        {
            h = length * sin(((double)i / (double)stacks) * (pi / 2));
            r = length * cos(((double)i / (double)stacks) * (pi / 2));
            for (j = 0; j <= slices; j++)
            {
                points[i][j].x = r * cos(((double)j / (double)slices) * 2 * pi);
                points[i][j].y = r * sin(((double)j / (double)slices) * 2 * pi);
                points[i][j].z = h;
            }
        }

        for (i = 0; i < stacks; i++)
        {
            glPushMatrix();
            glTranslatef(reference_point.x, reference_point.y, reference_point.z);
            glColor3f(color.r, color.g, color.b);
            for (j = 0; j < slices; j++)
            {
                glBegin(GL_QUADS);
                {

                    glVertex3f(points[i][j].x, points[i][j].y, points[i][j].z);
                    glVertex3f(points[i][j + 1].x, points[i][j + 1].y, points[i][j + 1].z);
                    glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, points[i + 1][j + 1].z);
                    glVertex3f(points[i + 1][j].x, points[i + 1][j].y, points[i + 1][j].z);

                    glVertex3f(points[i][j].x, points[i][j].y, -points[i][j].z);
                    glVertex3f(points[i][j + 1].x, points[i][j + 1].y, -points[i][j + 1].z);
                    glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, -points[i + 1][j + 1].z);
                    glVertex3f(points[i + 1][j].x, points[i + 1][j].y, -points[i + 1][j].z);
                }
                glEnd();
            }
            glPopMatrix();
        }
    }

    virtual double getT(Ray ray, Color &color, int level)
    {
        ray.origin = ray.origin - reference_point;

        double originDot = ray.origin.dot(ray.origin);
        double tp = -ray.dir.dot(ray.origin);
        double d2 = originDot - tp * tp;

        if (d2 > length * length)
            return -1;

        double t_ = length * length - d2;
        double t = tp - sqrt(t_);

        return t < 0 ? -1 : t;
    }

    friend std::istream &operator>>(std::istream &in, Sphere &s)
    {
        in >> s.reference_point >> s.length;
        in >> s.color.r >> s.color.g >> s.color.b;
        for (int i = 0; i < 4; i++)
            in >> s.coefficients[i];
        in >> s.shine;
        return in;
    }
};

struct CheckerBoard : public Object
{

    int tiles = 0;

    CheckerBoard()
    {
    }

    CheckerBoard(int floorWidth, int tileWidth)
    {
        tiles = floorWidth / tileWidth;
        reference_point = vect(-floorWidth / 2, -floorWidth / 2, 0);
        length = tileWidth;
    }

    virtual Color getColorAt(vect point)
    {
        int x_pos = (point.x - reference_point.x) / length;
        int y_pos = (point.y - reference_point.y) / length;

        if (x_pos < 0 || x_pos >= tiles || y_pos < 0 || y_pos >= tiles)
            return Color(0, 0, 0);

        if (((x_pos + y_pos) % 2) == 0)
            return Color(1, 1, 1);
        else
            return Color(0, 0, 0);
    }

    virtual Ray getNormal(vect point, Ray incidentRay)
    {
        if (incidentRay.dir.z > 0)
            return Ray(point, vect(0, 0, 1));
        else
            return Ray(point, vect(0, 0, -1));
    }

    virtual void draw()
    {
        for (int i = 0; i < tiles; i++)
        {
            for (int j = 0; j < tiles; j++)
            {
                if (((i + j) % 2) == 0)
                    glColor3f(1, 1, 1);
                else
                    glColor3f(0, 0, 0);

                glBegin(GL_QUADS);
                {
                    glVertex3f(reference_point.x + i * length, reference_point.y + j * length, 0);
                    glVertex3f(reference_point.x + (i + 1) * length, reference_point.y + j * length, 0);
                    glVertex3f(reference_point.x + (i + 1) * length, reference_point.y + (j + 1) * length, 0);
                    glVertex3f(reference_point.x + i * length, reference_point.y + (j + 1) * length, 0);
                }
                glEnd();
            }
        }
    }

    virtual double getT(Ray ray, Color &color, int level)
    {
        vect normal = vect(0, 0, 1);
        double t = -(normal.dot(ray.origin)) / normal.dot(ray.dir);
        vect p = ray.origin + ray.dir * t;
        return t;
    }
};

struct Plane : public Object
{
    vect a, b, c, d;

    Plane()
    {
    }

    Plane(vect a, vect b, vect c, vect d)
    {
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }

    virtual Ray getNormal(vect point, Ray incidentRay)
    {
        vect normal = (b - a) * (c - a);
        normal.normalize();
        if ((incidentRay.origin - this->reference_point).dot(normal) > 0)
            normal = normal * -1;
        return Ray(point, normal);
    }

    virtual void draw()
    {
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_QUADS);
        {
            glVertex3f(a.x, a.y, a.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(c.x, c.y, c.z);
            glVertex3f(d.x, d.y, d.z);
        }
        glEnd();
    }

    virtual double getT(Ray ray, Color &color, int level)
    {
        vect normal = (b - a) * (c - a);
        normal.normalize();

        double dotP = normal.dot(ray.dir);
        if (fabs(dotP) < EPS)
            return -1;

        double t = -(normal.dot(ray.origin - a)) / dotP;
        vect p = ray.origin + ray.dir * t;

        vect ab = b - a;
        vect bc = c - b;
        vect cd = d - c;
        vect da = a - d;

        vect ap = p - a;
        vect bp = p - b;
        vect cp = p - c;
        vect dp = p - d;

        if ((ab * ap).dot(normal) > 0 && (bc * bp).dot(normal) > 0 && (cd * cp).dot(normal) > 0 && (da * dp).dot(normal) > 0)
            return t;
        return -1;
    }
};
