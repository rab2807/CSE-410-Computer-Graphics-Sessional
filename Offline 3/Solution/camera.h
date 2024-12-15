#define pi (2 * acos(0.0))

struct Camera
{
    vect eye;
    vect up, look, right;
    double mov_angle;
    double mov_speed;

    Camera(double eye_x, double eye_y, double eye_z)
    {
        eye = vect(eye_x, eye_y, eye_z);
        look = {-1, 0, 0};
        right = {0, 1, 0};
        up = {0, 0, 1};
        mov_angle = 5;
        mov_speed = 5;
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
        vect temp = eye;
        eye = eye + up * mov_speed;
        vect a = temp - eye;
        vect b = look - temp;
        look = a + b;
        look.normalize();
        up = right * look;
    }

    void moveDownWithoutChangingReference()
    {
        vect temp = eye;
        eye = eye - up * mov_speed;
        vect a = temp - eye;
        vect b = look - temp;
        look = a + b;
        look.normalize();
        up = right * look;
    }

    // rotate from unit_vec1 to unit_vec2
    void rotate_axis(vect &unit_vec1, vect &unit_vec2, double mov_angle)
    {
        double radian_angle = mov_angle * pi / 180;
        vect temp = unit_vec1;
        unit_vec1 = unit_vec1 * cos(radian_angle) + unit_vec2 * sin(radian_angle);
        unit_vec2 = temp * (-sin(radian_angle)) + unit_vec2 * cos(radian_angle);

        // printf("eye: %lf %lf %lf\n", eye.x, eye.y, eye.z);
    }
};
