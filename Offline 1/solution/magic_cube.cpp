#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glut.h>
#include "camera.h"

#define pi 3.1416
#define STEPS 25

int drawgrid;
int drawaxes;
double angle;
double angleSpeed = 5;

Camera camera(5, 0, 2);
Point baseTriangleCenter = {0, 0, 0};
Point baseTriangleCentroid = {1.0 / 3, 1.0 / 3, 1.0 / 3};
double baseTriangleScale = 1;

double sphereRadius = 1.0 / sqrt(3);
double sphereFaceScale = 0;
double spherePositionX = 1;

double maxCylinderHeight = sqrt(2);
double cylinderMaxTranslationX = 1.0 / sqrt(2);
double cylinderHeight = maxCylinderHeight;
double cylinderScale = 0;
double cylinderPositionX = cylinderMaxTranslationX;

void init()
{
    // codes for initialization
    drawgrid = 0;
    drawaxes = 1;
    angle = 0;

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

void tranformToSphere()
{
    if (baseTriangleScale > 0.0)
    {
        // make the triangles smaller
        baseTriangleScale -= 1.0 / STEPS;
        baseTriangleCenter = baseTriangleCenter + baseTriangleCentroid / STEPS;

        // make the sphere faces bigger
        sphereFaceScale += 1.0 / STEPS;
        spherePositionX -= 1.0 / STEPS;

        // reduce cylinder height and translate it
        cylinderHeight -= maxCylinderHeight / STEPS;
        cylinderPositionX -= cylinderMaxTranslationX / STEPS;
    }
}

void transformToOctahedron()
{
    if (baseTriangleScale < 1.0)
    {
        // make the triangles bigger
        baseTriangleScale += 1.0 / STEPS;
        baseTriangleCenter = baseTriangleCenter - baseTriangleCentroid / STEPS;

        // make the sphere faces smaller
        sphereFaceScale -= 1.0 / STEPS;
        spherePositionX += 1.0 / STEPS;

        // increase cylinder height and translate it
        cylinderHeight += maxCylinderHeight / STEPS;
        cylinderPositionX += cylinderMaxTranslationX / STEPS;
    }
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
    case '.':
        transformToOctahedron();
        break;
    case ',':
        tranformToSphere();
        break;
    case 'w':
        camera.moveUpWithoutChangingReference();
        break;
    case 's':
        camera.moveDownWithoutChangingReference();
        break;
    case 'a':
        angle -= angleSpeed;
        break;
    case 'd':
        angle += angleSpeed;
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
{
    // x, y is the x-y of the screen (2D)
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
            //........
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

// ----- Octahedron -----
void drawBaseTriangle()
{
    glBegin(GL_TRIANGLES);
    {
        glVertex3f(1, 0, 0);
        glVertex3f(0, 1, 0);
        glVertex3f(0, 0, 1);
    }
    glEnd();
}

void drawUpperOctahedron()
{
    for (int i = 0; i < 4; i++)
    {
        if (i % 2 == 0)
            glColor3f(1, 0, 0);
        else
            glColor3f(0, 1, 0);
        glPushMatrix();
        glRotatef(90 * i, 0, 0, 1);
        glTranslated(baseTriangleCenter.x, baseTriangleCenter.y, baseTriangleCenter.z);
        glScaled(baseTriangleScale, baseTriangleScale, baseTriangleScale);
        glScaled(1, 1, 1);
        drawBaseTriangle();
        glPopMatrix();
    }
}

void drawFullOctahedron()
{
    glPushMatrix();
    drawUpperOctahedron();
    glRotated(180, 1, 0, 0);
    drawUpperOctahedron();
    glPopMatrix();
}

// ----- Sphere -----
void drawSphereFace(int slices)
{
    const float DEG2RAD = acos(-1) / 180.0f;

    Point n1; // normal of longitudinal plane rotating along Y-axis
    Point n2; // normal of latitudinal plane rotating along Z-axis
    Point v;  // direction vector intersecting 2 planes, n1 x n2
    float a1; // longitudinal angle along Y-axis
    float a2; // latitudinal angle along Z-axis

    // compute the number of vertices per row, 2^n + 1
    int pointsPerRow = (int)pow(2, slices) + 1;
    // std::cout << "pointsPerRow: " << pointsPerRow << std::endl;

    struct Point points[pointsPerRow][pointsPerRow];

    // rotate latitudinal plane from 45 to -45 degrees along Z-axis (top-to-bottom)
    for (unsigned int i = 0; i < pointsPerRow; ++i)
    {
        // normal for latitudinal plane
        // if latitude angle is 0, then normal vector of latitude plane is n2=(0,1,0)
        // therefore, it is rotating (0,1,0) vector by latitude angle a2
        a2 = DEG2RAD * (45.0f - 90.0f * i / (pointsPerRow - 1));
        n2.x = -sin(a2);
        n2.y = cos(a2);
        n2.z = 0;

        // rotate longitudinal plane from -45 to 45 along Y-axis (left-to-right)
        for (unsigned int j = 0; j < pointsPerRow; ++j)
        {
            // normal for longitudinal plane
            // if longitude angle is 0, then normal vector of longitude is n1=(0,0,-1)
            // therefore, it is rotating (0,0,-1) vector by longitude angle a1
            a1 = DEG2RAD * (-45.0f + 90.0f * j / (pointsPerRow - 1));
            n1.x = -sin(a1);
            n1.y = 0;
            n1.z = -cos(a1);

            // find direction vector of intersected line, n1 x n2
            v.x = n1.y * n2.z - n1.z * n2.y;
            v.y = n1.z * n2.x - n1.x * n2.z;
            v.z = n1.x * n2.y - n1.y * n2.x;

            // normalize direction vector and then multiply by radius
            float scale = sphereRadius / sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
            v = v * scale;

            points[i][j] = v;
        }
    }

    glBegin(GL_QUADS);
    for (int j = 0; j < pointsPerRow - 1; j++)
    {
        for (int i = 0; i < pointsPerRow - 1; i++)
        {
            glVertex3f(points[j][i].x, points[j][i].y, points[j][i].z);
            glVertex3f(points[j][i + 1].x, points[j][i + 1].y, points[j][i + 1].z);

            glVertex3f(points[j + 1][i + 1].x, points[j + 1][i + 1].y, points[j + 1][i + 1].z);
            glVertex3f(points[j + 1][i].x, points[j + 1][i].y, points[j + 1][i].z);
        }
    }
    glEnd();
}

void drawSphereParts()
{
    for (int i = 0; i < 4; i++)
    {
        if (i % 2 == 0)
            glColor3f(220.0 / 255, 40.0 / 255, 60.0 / 255);
        else
            glColor3f(95.0 / 255, 60.0 / 255, 220.0 / 255);
        glPushMatrix();
        glRotatef(90 * i, 0, 0, 1);
        glTranslated(spherePositionX, 0, 0);
        glScaled(sphereFaceScale, sphereFaceScale, sphereFaceScale);
        drawSphereFace(5);
        glPopMatrix();
    }

    for (int i = 0; i < 2; i++)
    {
        if (i % 2 == 0)
            glColor3f(.5, 0, 1);
        else
            glColor3f(1, .5, 0);
        glPushMatrix();
        glRotatef(90 + 180 * i, 0, 1, 0);
        glTranslated(spherePositionX, 0, 0);
        glScaled(sphereFaceScale, sphereFaceScale, sphereFaceScale);
        drawSphereFace(5);
        glPopMatrix();
    }
}

// ----- Cylinder -----
void drawCylinderFace(int segments)
{
    const float DEG2RAD = acos(-1) / 180.0f;

    struct Point points[segments + 1];

    double phi = acos(-1 / 3.0);
    double offset = (180 - phi) * DEG2RAD;

    for (int i = 0; i < segments + 1; i++)
    {
        double theta = -offset / 2 + i * offset / segments;
        points[i].x = sphereRadius * cos(theta);
        points[i].y = sphereRadius * sin(theta);
    }

    glBegin(GL_QUADS);
    for (int i = 0; i < segments; i++)
    {
        glVertex3f(points[i].x, points[i].y, cylinderHeight / 2);
        glVertex3f(points[i].x, points[i].y, -cylinderHeight / 2);
        glVertex3f(points[i + 1].x, points[i + 1].y, -cylinderHeight / 2);
        glVertex3f(points[i + 1].x, points[i + 1].y, cylinderHeight / 2);
    }
    glEnd();
}

void drawCylinderSet()
{
    double angle = 0;

    for (int i = 0; i < 4; i++)
    {
        glRotated(angle, 0, 1, 0);
        glPushMatrix();
        glRotated(-45, 0, 1, 0);
        glTranslated(cylinderPositionX, 0, 0);
        glScaled(sphereFaceScale, sphereFaceScale, 1);
        drawCylinderFace(50);
        glPopMatrix();
        angle += 90;
    }
}

void drawCylinderParts()
{
    glColor3f(220.0 / 255, 150.0 / 255, 90.0 / 255);
    drawCylinderSet();

    glColor3f(150.0 / 255, 90.0 / 255, 220.0 / 255);
    glPushMatrix();
    glRotated(90, 0, 0, 1);
    drawCylinderSet();
    glPopMatrix();

    glColor3f(90.0 / 255, 220.0 / 255, 150.0 / 255);
    glPushMatrix();
    glRotated(90, 1, 0, 0);
    drawCylinderSet();
    glPopMatrix();
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

    glPushMatrix();
    glRotated(angle, 0, 0, 1);
    drawFullOctahedron();
    drawSphereParts();
    drawCylinderParts();
    glPopMatrix();

    // ADD this line in the end --- if you use double buffer (i.e. GL_DOUBLE)
    glutSwapBuffers();
}

void animate()
{
    // angle += 0.05;
    // codes for any changes in Models, Camera
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

    glutMainLoop(); // The main loop of OpenGL

    return 0;
}
