#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <GL/glut.h>
#include "camera.h"

#define pi 3.1416
#define STEPS 25
#define PLANE_NUM 4
#define TIME_DRIVEN 0

int drawgrid;
int drawaxes;

Camera camera(8, 0, 4);

double ballRadius = 0.5;
double ballAngle = 0;
double ballRotationSpeed;
double ballMoveSpeed = 5;
Point ballPosition = {0, 0, ballRadius};
Point ballLookAt;
Point ballRight;
Point ballUp = {0, 0, 1};

Plane planes[] = {
    // left-right
    Plane({-7, -7, 0}, {-7, -7, 1.7}, {-3, 7, 1.7}, {-3, 7, 0}),
    Plane({7, -7, 0}, {7, -7, 1.7}, {7, 7, 1.7}, {7, 7, 0}),
    // front-back
    Plane({-7, -7, 0}, {-7, -7, 1.7}, {7, -7, 1.7}, {7, -7, 0}),
    Plane({-7, 7, 0}, {-7, 7, 1.7}, {7, 7, 1.7}, {7, 7, 0}),
};

bool animation = false;             // if simulation is happening
double manualMovementFactor = 0.05; // because simulation values doesnt work

double arrowHeight = 1;
double arrowRotationSpeed = 10;

int pastTime = 0;
int currentTime = 0;
int timeDiff;            // used for simulation ball movement and collision prediction
int latestEventTime = 0; // timestamp of the latest event
int collisionPlane;      // the plane where latest collision will occur

void timer(int m);

// get the plane with which the ball will collide first
void getCollisionPlane()
{
    double distance = __INT_MAX__;
    int targetTime, timeStamp;

    for (int i = 0; i < PLANE_NUM; i++)
    {
        Point intersectionPoint = planes[i].get_intersection(ballPosition, ballPosition + ballLookAt, -ballRadius);
        if (intersectionPoint.distance(ballPosition) < distance)
        {
            distance = intersectionPoint.distance(ballPosition);
            collisionPlane = i;
            targetTime = (int)(distance / ballMoveSpeed * 1000);
            timeStamp = glutGet(GLUT_ELAPSED_TIME);
        }
    }
    if (TIME_DRIVEN == 0)
        glutTimerFunc(targetTime, timer, timeStamp);
    latestEventTime = timeStamp;
}

void init()
{
    // codes for initialization
    drawgrid = 0;
    drawaxes = 1;

    ballLookAt = {-1.5, -1, 0};
    ballLookAt.normalize();
    ballRight = ballLookAt * ballUp;
    ballRight.normalize();

    pastTime = glutGet(GLUT_ELAPSED_TIME);

    getCollisionPlane();

    // clear the screen
    glClearColor(0, 0, 0, 0);

    /************************
    / set-up projection here
    ************************/
    // load the PROJECTION matrix
    glMatrixMode(GL_PROJECTION);

    // initialize the matrix
    glLoadIdentity();

    // give PERSPECTIVE parameters
    gluPerspective(80, 1, 1, 1000.0);
    // field of view in the Y (vertically)
    // aspect ratio that determines the field of view in the X direction (horizontally)
    // near distance
    // far distance
}

void drawAxes()
{
    // get window width
    int width = 25;
    int height = 25;
    if (drawaxes == 1)
    {
        glBegin(GL_LINES);
        {
            glColor3f(217.0 / 255, 158.0 / 255, 130.0 / 255);
            glVertex3f(width, 0, 0);
            glVertex3f(-width, 0, 0);

            glColor3f(175.0 / 255, 15.0 / 255, 215.0 / 255);
            glVertex3f(0, -width, 0);
            glVertex3f(0, width, 0);

            glColor3f(15.0 / 255, 215.0 / 255, 175.0 / 255);
            glVertex3f(0, 0, width);
            glVertex3f(0, 0, -width);
        }
        glEnd();
    }
}

void drawGrid()
{
    int i;
    if (drawgrid == 1)
    {
        glColor3f(0.6, 0.6, 0.6); // grey
        glBegin(GL_LINES);
        {
            for (i = -8; i <= 8; i++)
            {

                if (i == 0)
                    continue; // SKIP the MAIN axes

                // lines parallel to Y-axis
                glVertex3f(i * 10, -90, 0);
                glVertex3f(i * 10, 90, 0);

                // lines parallel to X-axis
                glVertex3f(-90, i * 10, 0);
                glVertex3f(90, i * 10, 0);

                // lines parallel to Z-axis
                glVertex3f(-90, 0, i * 10);
                glVertex3f(90, 0, i * 10);
            }
        }
        glEnd();
    }
}

// --- chess board ---
void drawChessBoard(int width, int height, int squares)
{
    double x, y;

    x = width / squares;
    y = height / squares;

    for (int i = 0; i < squares; i++)
    {
        for (int j = 0; j < squares; j++)
        {
            if ((i + j) % 2 == 0)
                glColor3f(0, 0, 0);
            else
                glColor3f(1, 1, 1);

            glBegin(GL_QUADS);
            {
                glVertex3f(-width / 2 + i * x, -height / 2 + j * y, 0);
                glVertex3f(-width / 2 + i * x + x, -height / 2 + j * y, 0);
                glVertex3f(-width / 2 + i * x + x, -height / 2 + j * y + y, 0);
                glVertex3f(-width / 2 + i * x, -height / 2 + j * y + y, 0);
            }
            glEnd();
        }
    }
}

// --- arrow ---
void drawArrow()
{
    glColor3f(0, 0, 1);
    glPushMatrix();
    glTranslated(0, 0, arrowHeight);
    glutSolidCone(0.1, 0.5, 5, 5);
    glPopMatrix();

    glLineWidth(3);
    glBegin(GL_LINES);
    {
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, arrowHeight);
    }
    glEnd();
    glLineWidth(1);
}

void repositionArrow()
{
    double angle = acos(ballLookAt.dot({1, 0, 0})) * 180 / pi; // angle between ballLookAt and x-axis
    if (ballLookAt.y < 0)
        angle = -angle;
    glPushMatrix();
    glTranslated(ballPosition.x, ballPosition.y, ballPosition.z);
    glRotated(angle, 0, 0, 1);
    glRotated(90, 0, 1, 0);
    drawArrow();
    glPopMatrix();
}

// --- sphere ---
void drawSphere(double radius, int slices, int stacks)
{
    struct Point points[100][100];
    int i, j;
    double h, r;
    // generate points
    for (i = 0; i <= stacks; i++)
    {
        h = radius * sin(((double)i / (double)stacks) * (pi / 2));
        r = radius * cos(((double)i / (double)stacks) * (pi / 2));
        for (j = 0; j <= slices; j++)
        {
            points[i][j].x = r * cos(((double)j / (double)slices) * 2 * pi);
            points[i][j].y = r * sin(((double)j / (double)slices) * 2 * pi);
            points[i][j].z = h;
        }
    }
    // draw quads using generated points
    for (i = 0; i < stacks; i++)
    {
        for (j = 0; j < slices; j++)
        {
            glBegin(GL_QUADS);
            {
                // upper hemisphere
                if (j % 2 == 0)
                    glColor3f(1, 0, 0);
                else
                    glColor3f(0, 1, 0);

                glVertex3f(points[i][j].x, points[i][j].y, points[i][j].z);
                glVertex3f(points[i][j + 1].x, points[i][j + 1].y, points[i][j + 1].z);
                glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, points[i + 1][j + 1].z);
                glVertex3f(points[i + 1][j].x, points[i + 1][j].y, points[i + 1][j].z);

                // lower hemisphere
                if (j % 2 != 0)
                    glColor3f(1, 0, 0);
                else
                    glColor3f(0, 1, 0);

                glVertex3f(points[i][j].x, points[i][j].y, -points[i][j].z);
                glVertex3f(points[i][j + 1].x, points[i][j + 1].y, -points[i][j + 1].z);
                glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, -points[i + 1][j + 1].z);
                glVertex3f(points[i + 1][j].x, points[i + 1][j].y, -points[i + 1][j].z);
            }
            glEnd();
        }
    }
}

void rotateDirection(bool clockwise)
{
    camera.rotate_axis(ballLookAt, ballRight, arrowRotationSpeed * (clockwise ? -1 : 1));
}

void drawBall()
{
    double angle = acos(ballLookAt.dot({1, 0, 0})) * 180 / pi;
    if (ballLookAt.y < 0)
        angle = -angle;

    glPushMatrix();
    glTranslated(ballPosition.x, ballPosition.y, ballPosition.z);
    // glRotated(-angle, 0, 0, 1);
    glRotated(-ballAngle, ballRight.x, ballRight.y, ballRight.z);
    drawSphere(ballRadius, 10, 50);
    glPopMatrix();
}

void keyboardListener(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1':
        camera.look_left();
        break;
    case '2':
        camera.look_right();
        break;
    case '3':
        camera.look_up();
        break;
    case '4':
        camera.look_down();
        break;
    case '5':
        camera.tilt_clockwise();
        break;
    case '6':
        camera.tilt_anticlockwise();
        break;
    case 'w':
        camera.moveUpWithoutChangingReference();
        break;
    case 's':
        camera.moveDownWithoutChangingReference();
        break;
    case 'i':
    {
        if (!animation)
        {
            getCollisionPlane();
            ballPosition = ballPosition + ballLookAt * ballMoveSpeed * manualMovementFactor;
            ballRotationSpeed = ballMoveSpeed / (2 * pi * ballRadius) * 360 * manualMovementFactor;
            ballAngle += ballRotationSpeed;
        }
        break;
    }
    case 'k':
    {
        getCollisionPlane();
        ballPosition = ballPosition - ballLookAt * ballMoveSpeed * manualMovementFactor;
        ballRotationSpeed = ballMoveSpeed / (2 * pi * ballRadius) * 360 * manualMovementFactor;
        ballAngle -= ballRotationSpeed;
        break;
    }
    case 'j':
        getCollisionPlane();
        rotateDirection(false);
        break;
    case 'l':
        getCollisionPlane();
        rotateDirection(true);
        break;
    case ' ':
        getCollisionPlane();
        animation = !animation;
        break;

    default:
        break;
    }
}

void specialKeyListener(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_DOWN:
        camera.move_forward();
        break;
    case GLUT_KEY_UP:
        camera.move_backward();
        break;
    case GLUT_KEY_RIGHT:
        camera.move_right();
        break;
    case GLUT_KEY_LEFT:
        camera.move_left();
        break;
    case GLUT_KEY_PAGE_UP:
        camera.move_up();
        break;
    case GLUT_KEY_PAGE_DOWN:
        camera.move_down();
        break;
    case GLUT_KEY_INSERT:
        camera.look_up();
        break;

    default:
        break;
    }
}

void mouseListener(int button, int state, int x, int y)
{ // x, y is the x-y of the screen (2D)
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        { // 2 times?? in ONE click? -- solution is checking DOWN or UP
            drawaxes = 1 - drawaxes;
        }
        break;

    case GLUT_RIGHT_BUTTON:
        //........
        break;

    case GLUT_MIDDLE_BUTTON:
        //........
        break;

    default:
        break;
    }
}

void display()
{

    // clear the display
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0, 0, 0, 0); // color black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /********************
    / set-up camera here
    ********************/
    // load the correct matrix -- MODEL-VIEW matrix
    glMatrixMode(GL_MODELVIEW);

    // initialize the matrix
    glLoadIdentity();

    gluLookAt(camera.eye.x, camera.eye.y, camera.eye.z,
              camera.eye.x + camera.look.x, camera.eye.y + camera.look.y, camera.eye.z + camera.look.z,
              camera.up.x, camera.up.y, camera.up.z);

    // again select MODEL-VIEW
    glMatrixMode(GL_MODELVIEW);

    /****************************
    / Add your objects from here
    ****************************/
    // add objects

    drawAxes();
    // drawGrid();

    drawChessBoard(80, 80, 80);

    repositionArrow();

    drawBall();

    for (int i = 0; i < PLANE_NUM; i++)
    {
        // double col = 1 - i * 0.25;
        // glColor3f(col, col, col);
        glColor3f(1, 0, 0);
        planes[i].draw();
    }

    // ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
    glutSwapBuffers();
}

void animate()
{
    // recording time difference from previous frame
    currentTime = glutGet(GLUT_ELAPSED_TIME);
    timeDiff = currentTime - pastTime;
    pastTime = currentTime;

    // ball movement
    if (animation)
    {
        ballPosition = ballPosition + ballLookAt * ballMoveSpeed * timeDiff / 1000;
        ballRotationSpeed = ballMoveSpeed / (2 * pi * ballRadius) * 360 * timeDiff / 1000;
        ballAngle += ballRotationSpeed;
    }

    // collision detection [time-driven]
    if (TIME_DRIVEN == 1)
    {
        for (int i = 0; i < PLANE_NUM; i++)
        {
            double distance = planes[i].get_distance(ballPosition);
            // printf("distance: %lf\n", distance);
            if (distance < ballRadius)
            {
                printf("Collision with plane %d\n", i);
                getCollisionPlane();
                Point normal = planes[i].get_normal();
                ballLookAt = ballLookAt - normal * (ballLookAt.dot(normal)) * 2;
                ballLookAt.normalize();
                ballRight = ballLookAt * ballUp;
                ballRight.normalize();
                break;
            }
        }
    }
    glutPostRedisplay();
}

void timer(int timeStamp)
{
    if (timeStamp < latestEventTime)
        return;
    double distance = planes[collisionPlane].get_distance(ballPosition);
    if (distance > ballRadius)
    {
        getCollisionPlane(); // if the ball is idle, again get the collision plane
        return;
    }

    // collision detection [event-driven]
    printf("Collision with plane %d\n", collisionPlane);
    getCollisionPlane();
    Point normal = planes[collisionPlane].get_normal();
    ballLookAt = ballLookAt - normal * (ballLookAt.dot(normal)) * 2;
    ballLookAt.normalize();
    ballRight = ballLookAt * ballUp;
    ballRight.normalize();

    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB); // Depth, Double buffer, RGB color

    glutCreateWindow("My OpenGL Program");

    init();

    glEnable(GL_DEPTH_TEST); // enable Depth Testing

    glutDisplayFunc(display); // display callback function
    glutIdleFunc(animate);    // what you want to do in the idle time (when no drawing is occuring)

    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);
    glutMouseFunc(mouseListener);
    glutTimerFunc(1000, timer, 0);

    glutMainLoop(); // The main loop of OpenGL

    return 0;
}
