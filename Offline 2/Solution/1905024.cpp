#include "mat.h"
#include "vec.h"
#include "bitmap_image.hpp"
#include <bits/stdc++.h>
using namespace std;

static unsigned long int g_seed = 1;
inline int random()
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}

struct Triangle
{
    vect vtx[3];
    int r, g, b;

    Triangle()
    {
        // random color
        r = random() % 256;
        g = random() % 256;
        b = random() % 256;
    }
};

vect eye, l, r, u;
double fovY, aspectRatio, near, far;

double screen_Width, screen_Height;
bitmap_image image;

double pi = 2 * acos(0.0);
double **z_buff;
vector<Triangle> triangles;

stack<matrix> matrix_stack;
matrix current_matrix(4, 4);

double right_limit = 1, left_limit = -1, top_limit = 1, bottom_limit = -1, front_limit = -1, back_limit = 1;
double dx, dy, Top_Y, Bottom_Y, Left_X, Right_X;
double z_max = 1;

ofstream outfile1, outfile2, outfile3, outfile4;
ifstream infile, config_file;

void read_input(string in_name, string config_name)
{
    infile.open(in_name);
    config_file.open(config_name);

    vect look, up;
    infile >> eye.x >> eye.y >> eye.z;
    infile >> look.x >> look.y >> look.z;
    infile >> up.x >> up.y >> up.z;
    infile >> fovY >> aspectRatio >> near >> far;

    l = (look - eye).normalize();
    r = (l.cross(up)).normalize();
    u = (r.cross(l)).normalize();

    config_file >> screen_Width >> screen_Height;

    image.setwidth_height(screen_Width, screen_Height);
    image.set_all_channels(0, 0, 0); // black background

    outfile1.open("stage1.txt");
    outfile2.open("stage2.txt");
    outfile3.open("stage3.txt");
    outfile4.open("z_buffer.txt");
}

void translate(double tx, double ty, double tz)
{
    matrix result(4, 4);
    result.mat[0][3] = tx;
    result.mat[1][3] = ty;
    result.mat[2][3] = tz;

    current_matrix = current_matrix * result;
    matrix_stack.pop();
    matrix_stack.push(current_matrix);
}

void scale(double sx, double sy, double sz)
{
    matrix result(4, 4);
    result.mat[0][0] = sx;
    result.mat[1][1] = sy;
    result.mat[2][2] = sz;

    current_matrix = current_matrix * result;
    matrix_stack.pop();
    matrix_stack.push(current_matrix);
}

vect rodrigues(vect p, vect a, double angle)
{
    angle = angle * (pi / 180);
    double c = cos(angle);
    double s = sin(angle);

    vect result = p * c + a.cross(p) * s + a * a.dot(p) * (1 - c);
    return result;
}

void rotate(double angle, vect axis)
{
    axis = axis.normalize();
    vect i(1, 0, 0), j(0, 1, 0), k(0, 0, 1);

    vect c1 = rodrigues(i, axis, angle);
    vect c2 = rodrigues(j, axis, angle);
    vect c3 = rodrigues(k, axis, angle);

    matrix result(4, 4);
    result.mat[0][0] = c1.x;
    result.mat[0][1] = c2.x;
    result.mat[0][2] = c3.x;

    result.mat[1][0] = c1.y;
    result.mat[1][1] = c2.y;
    result.mat[1][2] = c3.y;

    result.mat[2][0] = c1.z;
    result.mat[2][1] = c2.z;
    result.mat[2][2] = c3.z;

    current_matrix = current_matrix * result;
    matrix_stack.pop();
    matrix_stack.push(current_matrix);
}

void push()
{
    matrix_stack.push(current_matrix);
}

void pop()
{
    matrix_stack.pop();
    current_matrix = matrix_stack.top();
}

void model_transform()
{
    string command;
    while (infile >> command)
    {
        if (command == "triangle")
        {
            Triangle t;
            for (int i = 0; i < 3; i++)
            {
                double x, y, z;
                infile >> x >> y >> z;
                vect v(x, y, z);
                matrix vect_matrix = v.toMatrix();
                vect_matrix = current_matrix * vect_matrix;
                v = v.toVect(vect_matrix);
                v = v.homogeneous();
                t.vtx[i] = v;
            }
            triangles.push_back(t);

            for (int i = 0; i < 3; i++)
            {

                outfile1 << std::fixed << std::setprecision(7) << t.vtx[i].x << " " << t.vtx[i].y << " " << t.vtx[i].z << endl;
            }
            outfile1 << endl;
        }
        else if (command == "translate")
        {
            double tx, ty, tz;
            infile >> tx >> ty >> tz;
            translate(tx, ty, tz);
        }
        else if (command == "scale")
        {
            double sx, sy, sz;
            infile >> sx >> sy >> sz;
            scale(sx, sy, sz);
        }
        else if (command == "rotate")
        {
            double angle, x, y, z;
            infile >> angle >> x >> y >> z;
            vect axis(x, y, z);
            rotate(angle, axis);
        }
        else if (command == "push")
        {
            push();
        }
        else if (command == "pop")
        {
            pop();
        }
    }
    outfile1.close();
}

void view_transform()
{
    matrix T(4, 4);
    T.mat[0][3] = -eye.x;
    T.mat[1][3] = -eye.y;
    T.mat[2][3] = -eye.z;

    matrix R(4, 4);
    R.mat[0][0] = r.x;
    R.mat[0][1] = r.y;
    R.mat[0][2] = r.z;

    R.mat[1][0] = u.x;
    R.mat[1][1] = u.y;
    R.mat[1][2] = u.z;

    R.mat[2][0] = -l.x;
    R.mat[2][1] = -l.y;
    R.mat[2][2] = -l.z;

    matrix V = R * T;

    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            matrix vect_matrix = triangles[i].vtx[j].toMatrix();
            vect_matrix = V * vect_matrix;
            triangles[i].vtx[j] = triangles[i].vtx[j].toVect(vect_matrix);
            triangles[i].vtx[j] = triangles[i].vtx[j].homogeneous();

            outfile2 << std::fixed << std::setprecision(7) << triangles[i].vtx[j].x << " " << triangles[i].vtx[j].y << " " << triangles[i].vtx[j].z << endl;
        }
        outfile2 << endl;
    }
    outfile2.close();
}

void projection_transform()
{
    double fovX = fovY * aspectRatio;
    double t = near * tan((fovY / 2.0) * (pi / 180));
    double r = near * tan((fovX / 2.0) * (pi / 180));

    matrix P(4, 4, 0);
    P.mat[0][0] = near / r;
    P.mat[1][1] = near / t;
    P.mat[2][2] = -(far + near) / (far - near);
    P.mat[2][3] = -(2 * far * near) / (far - near);
    P.mat[3][2] = -1;

    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            matrix vect_matrix = triangles[i].vtx[j].toMatrix();
            vect_matrix = P * vect_matrix;
            triangles[i].vtx[j] = triangles[i].vtx[j].toVect(vect_matrix);
            triangles[i].vtx[j] = triangles[i].vtx[j].homogeneous();

            outfile3 << std::fixed << std::setprecision(7) << triangles[i].vtx[j].x << " " << triangles[i].vtx[j].y << " " << triangles[i].vtx[j].z << endl;
        }
        outfile3 << endl;
    }
    outfile3.close();
}

void initializeZBuffer()
{
    dx = (right_limit - left_limit) / screen_Width;
    dy = (top_limit - bottom_limit) / screen_Height;

    Top_Y = top_limit - (dy / 2);
    Bottom_Y = bottom_limit + (dy / 2);
    Left_X = left_limit + (dx / 2);
    Right_X = right_limit - (dx / 2);

    z_buff = new double *[(int)screen_Height];
    for (int i = 0; i < screen_Height; i++)
    {
        z_buff[i] = new double[(int)screen_Width];
        for (int j = 0; j < screen_Width; j++)
            z_buff[i][j] = z_max;
    }
}

#define det(a, b, c, d) (a * d - b * c)

void intersect(vect p1, vect p2, vect p3, vect p4, vect &intersection)
{
    // https://math.stackexchange.com/questions/3975073/neat-way-to-calculate-intersection-of-2-lines-using-4-points

    double x1 = p1.x, y1 = p1.y;
    double x2 = p2.x, y2 = p2.y;
    double x3 = p3.x, y3 = p3.y;
    double x4 = p4.x, y4 = p4.y;

    // cout << "(" << x1 << ", " << y1 << ") "
    //      << "(" << x2 << ", " << y2 << ") "
    //      << "(" << x3 << ", " << y3 << ") "
    //      << "(" << x4 << ", " << y4 << ") "
    //      << endl;

    double a1 = y2 - y1;
    double b1 = x1 - x2;
    double c1 = det(x1, y1, x2, y2);

    double a2 = y4 - y3;
    double b2 = x3 - x4;
    double c2 = det(x3, y3, x4, y4);

    double denom = det(a1, b1, a2, b2);
    if (denom == 0)
    {
        // cout << "parallel lines\n\n";
        intersection.x = INT_MAX;
        intersection.y = INT_MAX;
        return;
    }

    double numer1 = det(c1, b1, c2, b2);
    double numer2 = det(a1, c1, a2, c2);

    intersection.x = numer1 / denom;
    intersection.y = numer2 / denom;

    // cout << intersection.x << " " << intersection.y << "\n\n";
}

void find_intersect_col(Triangle tri, double y, double max_x, double min_x, double &xa, double &xb)
{
    vector<vect> intersections;

    for (int i = 0; i < 3; i++)
    {
        // an edge of the triangle
        vect p1 = tri.vtx[i];
        vect p2 = tri.vtx[(i + 1) % 3];

        // scanline
        vect p3 = vect(Left_X, y, 0);
        vect p4 = vect(Right_X, y, 0);

        // intersection of the edge with the scanline
        vect intersection;
        intersect(p1, p2, p3, p4, intersection);

        // if intersection is within the x limits of the triangle
        if (intersection.x > min_x - dx / 2 && intersection.x < max_x + dx / 2)
        {
            intersections.push_back(intersection);
            continue;
        }
    }

    if (intersections.size() != 2)
    {
        // if scanline passes through a vertex, we might get 3 intersections
        if (intersections.size() == 3)
        {
            xa = xb = intersections[0].x;
            for (vect v : intersections)
            {
                // find the intersection closest to the min_x and max_x
                if (abs(min_x - v.x) < abs(min_x - xa))
                    xa = v.x;
                if (abs(max_x - v.x) < abs(max_x - xb))
                    xb = v.x;
            }
            return;
        }

        cout << "ERROR: intersections.size() != 2. found " << intersections.size() << endl;
        exit(0);
    }

    int k = intersections[0].x < intersections[1].x ? 0 : 1;
    xa = intersections[k].x;     // left intersection
    xb = intersections[1 - k].x; // right intersection
}

void plane_equation_params(vect v1, vect v2, vect v3, double &A, double &B, double &C, double &D)
{
    vect normal = ((v2 - v1).cross(v3 - v1)).normalize();
    A = normal.x;
    B = normal.y;
    C = normal.z;
    D = -(A * v1.x + B * v1.y + C * v1.z);
}

void clip_scan(Triangle tri)
{
    double max_y = max(tri.vtx[0].y, max(tri.vtx[1].y, tri.vtx[2].y));
    double min_y = min(tri.vtx[0].y, min(tri.vtx[1].y, tri.vtx[2].y));
    double max_x = max(tri.vtx[0].x, max(tri.vtx[1].x, tri.vtx[2].x));
    double min_x = min(tri.vtx[0].x, min(tri.vtx[1].x, tri.vtx[2].x));

    // cout << max_x << " " << min_x << " " << max_y << " " << min_y << endl;

    double clipped_max_y = min(max_y, Top_Y);
    double clipped_min_y = max(min_y, Bottom_Y);
    double clipped_max_x = min(max_x, Right_X);
    double clipped_min_x = max(min_x, Left_X);

    // cout << clipped_max_x << " " << clipped_min_x << " " << clipped_max_y << " " << clipped_min_y << endl;

    double A, B, C, D;
    plane_equation_params(tri.vtx[0], tri.vtx[1], tri.vtx[2], A, B, C, D);
    double constant = -A / C;

    int top_scanline = ceil((Top_Y - clipped_max_y) / dy);
    int bottom_scanline = round((Top_Y - clipped_min_y) / dy);

    // cout << top_scanline << " " << bottom_scanline << endl;

    for (int scanline = top_scanline; scanline < bottom_scanline; scanline++)
    {
        double xa, xb;
        double y = Top_Y - (scanline * dy);
        find_intersect_col(tri, y, max_x, min_x, xa, xb);

        xa = max(xa, clipped_min_x);
        xb = min(xb, clipped_max_x);
        int leftcol = round((xa - Left_X) / dx);
        int rightcol = round((xb - Left_X) / dx);

        double z1 = -(A * (Left_X + leftcol * dx) + B * y + D) / C;

        for (int col = leftcol, k = 0; col <= rightcol; col++, k++)
        {
            double z = z1 + constant * k * dx;

            // update z-buffer
            if (z >= front_limit && z < z_buff[scanline][col])
            {
                z_buff[scanline][col] = z;
                image.set_pixel(col, scanline, tri.r, tri.g, tri.b);
            }
        }
    }
}

void zBufferOutput()
{
    for (int i = 0; i < triangles.size(); i++)
        clip_scan(triangles[i]);

    image.save_image("out.bmp");

    for (int i = 0; i < screen_Height; i++)
    {
        for (int j = 0; j < screen_Width; j++)
        {
            if (z_buff[i][j] < z_max)
                outfile4 << std::fixed << std::setprecision(6) << z_buff[i][j] << "\t";
        }
        outfile4 << endl;
    }
    outfile4.close();
}

void freeMemory()
{
    for (int i = 0; i < screen_Height; i++)
        delete[] z_buff[i];
    delete[] z_buff;
}

int main()
{
    string in_name = "scene.txt";
    string config_name = "config.txt";

    matrix_stack.push(current_matrix);

    read_input(in_name, config_name);
    model_transform();
    view_transform();
    projection_transform();
    initializeZBuffer();
    zBufferOutput();
    freeMemory();
    cout << "Done\n";
}