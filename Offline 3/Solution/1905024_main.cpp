#include <bits/stdc++.h>
#include <GL/glut.h>
#include <time.h>
using namespace std;
#include "1905024_classes.h"
#include "bitmap_image.hpp"
#include "camera.h"
#include <windows.h>
#define pi (2 * acos(0.0))

Camera camera(150, 30, 50);
int drawgrid;
int drawaxes;
double angle;

int recursionLevel;
int imageHeight, imageWidth;
bitmap_image image;

vector<Object *> objects;
vector<Light *> lights;
vector<SpotLight *> spotlights;

int imageCount = 11;
bool rtxOn = false;

// draw axes
void drawAxes()
{
	if (drawaxes == 1)
	{
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINES);
		{
			glVertex3f(100, 0, 0);
			glVertex3f(-100, 0, 0);

			glVertex3f(0, -100, 0);
			glVertex3f(0, 100, 0);

			glVertex3f(0, 0, 100);
			glVertex3f(0, 0, -100);
		}
		glEnd();
	}
}

double windowWidth = 500, windowHeight = 500;
double viewAngle = 80;
double planeDistance = (windowHeight / 2.0) / tan((pi * viewAngle) / (360.0));
double startTimer, endTimer;

void capture()
{
	startTimer = clock();

	for (int i = 0; i < imageWidth; i++)
		for (int j = 0; j < imageHeight; j++)
			image.set_pixel(i, j, 0, 0, 0);

	vect topLeft = camera.eye + (camera.look * planeDistance) + (camera.up * (windowHeight / 2.0)) - (camera.right * (windowWidth / 2.0));
	double du = windowWidth / (imageWidth * 1.0);
	double dv = windowHeight / (imageHeight * 1.0);
	topLeft = topLeft + (camera.right * du / 2.0) - (camera.up * dv / 2.0);

	int nearestObjectIndex = -1;
	double t, tMin;

	for (int i = 0; i < imageWidth; i++)
	{
		for (int j = 0; j < imageHeight; j++)
		{
			vect pixel = topLeft + (camera.right * du * i) - (camera.up * dv * j);
			Ray ray(camera.eye, pixel - camera.eye);
			Color color;
			tMin = -1;
			nearestObjectIndex = -1;
			for (int k = 0; k < objects.size(); k++)
			{
				t = objects[k]->intersect(ray, color, 0);
				if (t > 0 && (nearestObjectIndex == -1 || t < tMin))
				{
					tMin = t;
					nearestObjectIndex = k;
				}
			}

			if (nearestObjectIndex != -1)
			{
				color = Color(0, 0, 0);
				double t = objects[nearestObjectIndex]->intersect(ray, color, 1);
				image.set_pixel(i, j, 255 * color.r, 255 * color.g, 255 * color.b);
			}
		}
	}

	// store image.data() upside down into temp
	image.bgr_to_rgb();
	unsigned char *temp = new unsigned char[imageWidth * imageHeight * 3];
	for (int i = 0; i < imageHeight; i++)
		for (int j = 0; j < imageWidth; j++)
			for (int k = 0; k < 3; k++)
				temp[(i * imageWidth + j) * 3 + k] = image.data()[(imageWidth * (imageHeight - i - 1) + j) * 3 + k];

	// shrink the image to fit into the window
	glPixelZoom(windowWidth / imageWidth, windowHeight / imageHeight);

	glDrawPixels(imageWidth, imageHeight, GL_RGB, GL_UNSIGNED_BYTE, temp);
	image.rgb_to_bgr();

	endTimer = clock();
	cout << "Time taken: " << (endTimer - startTimer) / CLOCKS_PER_SEC << " seconds" << endl;
}

void save()
{
	cout << "saving......";
	capture();
	image.save_image("Output_" + to_string(imageCount) + ".bmp");
	imageCount++;
	cout << "saved!" << endl;
}

void keyboardListener(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '0':
		save();
		break;
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
	case ' ':
		rtxOn = !rtxOn;
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
	default:
		break;
	}
}

void mouseListener(int button, int state, int x, int y)
{ // x, y is the x-y of the screen (2D)
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		break;
	case GLUT_RIGHT_BUTTON:
		break;
	case GLUT_MIDDLE_BUTTON:
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

	// drawAxes();

	if (!rtxOn)
	{
		for (int i = 0; i < objects.size(); i++)
		{
			Object *object = objects[i];
			object->draw();
		}

		for (int i = 0; i < lights.size(); i++)
		{
			Light *light = lights[i];
			light->draw();
		}

		for (int i = 0; i < spotlights.size(); i++)
		{
			SpotLight *spotlight = spotlights[i];
			spotlight->draw();
		}
	}
	else
		capture();

	glutSwapBuffers();
}

void animate()
{
	// codes for any changes in Models, Camera
	glutPostRedisplay();
}

void loadData()
{
	ifstream in("scene.txt");
	in >> recursionLevel >> imageHeight;

	imageWidth = imageHeight;

	int n;
	in >> n;

	for (int i = 0; i < n; i++)
	{
		string objType;
		in >> objType;

		Object *obj;

		if (objType == "sphere")
		{
			obj = new Sphere();
			in >> *((Sphere *)obj);
		}
		else if (objType == "triangle")
		{
			obj = new Triangle();
			in >> *((Triangle *)obj);
		}
		else if (objType == "general")
		{
			obj = new General();
			in >> *((General *)obj);
		}
		else
		{
			cout << "Invalid object type" << endl;
			exit(0);
		}
		objects.push_back(obj);
	}

	cout << "Object count " << objects.size() << endl;

	int nPointLight;
	in >> nPointLight;

	cout << nPointLight << endl;

	for (int i = 0; i < nPointLight; i++)
	{
		Light *light = new Light();
		in >> *light;
		lights.push_back(light);
	}

	int nSpotLight;
	in >> nSpotLight;

	for (int i = 0; i < nSpotLight; i++)
	{
		SpotLight *spotlight = new SpotLight();
		in >> *spotlight;
		spotlights.push_back(spotlight);
	}

	Object *checkerBoard;
	checkerBoard = new CheckerBoard(600, 20);
	double coefficients[] = {0.4, 0.2, 0.2, 0.4};
	checkerBoard->setCoefficients(coefficients);
	objects.push_back(checkerBoard);

	// prism
	vector<vect> vertices = {vect(-50, -50, 50),
							 vect(-50, -20, 50),
							 vect(-50, -20, 30),
							 vect(-50, -50, 30),
							 vect(-50 + 20, -50, 50),
							 vect(-50 + 20, -20, 50)};
	Object *p1 = new Plane(vertices[0], vertices[1], vertices[2], vertices[3]);
	Object *p2 = new Plane(vertices[0], vertices[1], vertices[5], vertices[4]);
	Object *p3 = new Plane(vertices[4], vertices[5], vertices[2], vertices[3]);
	Object *t1 = new Triangle(vertices[0], vertices[3], vertices[4]);
	Object *t2 = new Triangle(vertices[1], vertices[2], vertices[5]);

	p1->setColor(Color(0.8, 0.8, 0.8));
	p2->setColor(Color(0.8, 0.8, 0.8));
	p3->setColor(Color(0.8, 0.8, 0.8));
	t1->setColor(Color(0.8, 0.8, 0.8));
	t2->setColor(Color(0.8, 0.8, 0.8));

	double coefficients1[4] = {0};
	p1->setCoefficients(coefficients1);
	p2->setCoefficients(coefficients1);
	p3->setCoefficients(coefficients1);
	t1->setCoefficients(coefficients1);
	t2->setCoefficients(coefficients1);
	p1->refractiveIndex = p2->refractiveIndex = p3->refractiveIndex = t1->refractiveIndex = t2->refractiveIndex = 1.52;
	p1->coefficients[4] = p2->coefficients[4] = p3->coefficients[4] = t1->coefficients[4] = t2->coefficients[4] = 1;

	// objects.push_back(p1);
	// objects.push_back(p2);
	// objects.push_back(p3);
	// objects.push_back(t1);
	// objects.push_back(t2);

	SpotLight *light = new SpotLight();
	light->pos = vect(-60, -35, 40);
	light->color = Color(1, 0, 0);
	light->dir = vect(-1, 0, 0);
	light->cutoffAngle = 15;
	// spotlights.push_back(light);
}

void init()
{
	// codes for initialization
	drawgrid = 1;
	drawaxes = 1;

	loadData();
	image = bitmap_image(imageWidth, imageHeight);

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

int main(int argc, char **argv)
{

	glutInit(&argc, argv);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB); // Depth, Double buffer, RGB color

	glutCreateWindow("Ray Tracing");

	init();

	glEnable(GL_DEPTH_TEST); // enable Depth Testing

	glutDisplayFunc(display); // display callback function
	glutIdleFunc(animate);	  // what you want to do in the idle time (when no drawing is occuring)

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);
	glutMouseFunc(mouseListener);

	glutMainLoop(); // The main loop of OpenGL

	return 0;
}
