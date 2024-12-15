#define pi 3.1416

struct Point
{
    Point() {}
    double x, y, z;

    Point(double x, double y, double z) : x(x), y(y), z(z) {}
    Point(const Point &p) : x(p.x), y(p.y), z(p.z) {}

    double length() { return sqrt(x * x + y * y + z * z); }
    void normalize() { *this = *this / length(); }
    double dot(Point b) { return x * b.x + y * b.y + z * b.z; }
    double distance(Point b) { return sqrt((x - b.x) * (x - b.x) + (y - b.y) * (y - b.y) + (z - b.z) * (z - b.z)); }

    // arithemtic operations
    Point operator+(Point b) { return Point(x + b.x, y + b.y, z + b.z); }
    Point operator-(Point b) { return Point(x - b.x, y - b.y, z - b.z); }
    Point operator*(double b) { return Point(x * b, y * b, z * b); }
    Point operator/(double b) { return Point(x / b, y / b, z / b); }
    Point operator*(Point b) { return Point(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
};

class Plane
{
public:
    Point a, b, c, d;

    Plane() {}

    Plane(Point a, Point b, Point c, Point d)
    {
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }

    void draw()
    {
        // glColor3f(1, 0, 0);
        glBegin(GL_QUADS);
        {
            glVertex3f(a.x, a.y, a.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(c.x, c.y, c.z);
            glVertex3f(d.x, d.y, d.z);
        }
        glEnd();
    }

    // get normal vector of the plane
    Point get_normal()
    {
        Point ab = b - a;
        Point ac = c - a;
        Point normal = ab * ac;
        return normal / normal.length();
    }

    // find distance of a point from the plane
    double get_distance(Point p)
    {
        Point normal = get_normal();
        return abs(normal.x * (p.x - a.x) + normal.y * (p.y - a.y) + normal.z * (p.z - a.z));
    }

    // intersection point of a line and a plane
    Point get_intersection(Point p, Point q, double offset = 0)
    {
        Point normal = get_normal();
        if (p.distance(normal) > p.distance(normal * -1.0)) // if normal is not facing the line
            offset *= -1;
        Point _a = a + normal * offset;
        double t = (normal.dot(_a) - normal.dot(p)) / normal.dot(q - p);
        return p + (q - p) * t;
    }
};

struct Camera
{
    Point eye;
    Point up, look, right;
    double mov_angle;
    double mov_speed;

    Camera(double eye_x, double eye_y, double eye_z)
    {
        eye = Point(eye_x, eye_y, eye_z);
        look = {-1, 0, 0};
        right = {0, 1, 0};
        up = {0, 0, 1};
        mov_angle = 1;
        mov_speed = 1;
    }

    void look_left()
    {
        rotate_axis(look, right, -mov_angle);
    }

    void look_right()
    {
        rotate_axis(look, right, mov_angle);
    }

    void look_up()
    {
        rotate_axis(look, up, mov_angle);
    }

    void look_down()
    {
        rotate_axis(look, up, -mov_angle);
    }

    void tilt_clockwise()
    {
        rotate_axis(up, right, mov_angle);
    }

    void tilt_anticlockwise()
    {
        rotate_axis(up, right, -mov_angle);
    }

    void move_forward()
    {
        eye = eye - look * mov_speed;
    }

    void move_backward()
    {
        eye = eye + look * mov_speed;
    }

    void move_left()
    {
        eye = eye - right * mov_speed;
    }

    void move_right()
    {
        eye = eye + right * mov_speed;
    }

    void move_up()
    {
        eye = eye + up * mov_speed;
    }

    void move_down()
    {
        eye = eye - up * mov_speed;
    }

    void moveUpWithoutChangingReference()
    {
        Point temp = eye;
        eye = eye + up * mov_speed;
        Point a = temp - eye;
        Point b = look - temp;
        look = a + b;
        look.normalize();
        up = right * look;
    }

    void moveDownWithoutChangingReference()
    {
        Point temp = eye;
        eye = eye - up * mov_speed;
        Point a = temp - eye;
        Point b = look - temp;
        look = a + b;
        look.normalize();
        up = right * look;
    }

    // rotate from unit_vec1 to unit_vec2
    void rotate_axis(Point &unit_vec1, Point &unit_vec2, double mov_angle)
    {
        double radian_angle = mov_angle * pi / 180;
        Point temp = unit_vec1;
        unit_vec1 = unit_vec1 * cos(radian_angle) + unit_vec2 * sin(radian_angle);
        unit_vec2 = temp * (-sin(radian_angle)) + unit_vec2 * cos(radian_angle);

        // printf("eye: %lf %lf %lf\n", eye.x, eye.y, eye.z);
    }
};
