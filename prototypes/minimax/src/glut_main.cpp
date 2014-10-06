#include <iostream>
#include <vector>
#include <cmath>
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

class Circle {
  GLfloat center_x, center_y, radius;
  int N_MAX_POINTS;

 public:
  Circle(void) {
    N_MAX_POINTS = 30;

    center_x = 0.0f;
    center_y = 0.0f;

    radius = 0.5f;
  }

  void setRadius(float r) {
    radius = r;
  }

  void setCenter(float x, float y) {
    center_x = x;
    center_y = y;
  }

  void print() {
    glBegin(GL_POLYGON);
      glColor3f(1.0f, 0.0f, 0.0f);
      glVertex2f( center_x, center_y);
      for(int i=0; i<=N_MAX_POINTS ;i++)
        glVertex2f( center_x + radius * cos(2.0 * M_PI * i / N_MAX_POINTS), center_y + radius * sin(2.0 * M_PI * i / N_MAX_POINTS));
    glEnd();
  }
};

class Field {
  GLfloat w,h;
  GLfloat goal_size, goal_w;

 public:
  Field() {
    w = 5.4f;
    h = 7.4f;

    goal_size = 0.7f;
    goal_w = 0.3f;
  }

  void print() {
    glBegin(GL_POLYGON);
      glColor3f(0.0f, 1.0f, 0.0f);
      glVertex2f(  w/2,  h/2);
      glVertex2f(  w/2, -h/2);
      glVertex2f( -w/2, -h/2);
      glVertex2f( -w/2,  h/2);
    glEnd();

    glBegin(GL_POLYGON);
      glColor3f(1.0f, 1.0f, 1.0f);
      glVertex2f( -w/2,  goal_size/2);
      glVertex2f( -w/2,  -goal_size/2);
      glVertex2f( -w/2 - goal_w, -goal_size/2);
      glVertex2f( -w/2 - goal_w,  goal_size/2);
    glEnd();
  }
};


void display() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
  glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer

  // Draw a Red 1x1 Square centered at origin
  Field campo;
  campo.print();

  Circle c;
  c.setRadius(.09);
  c.print();
  c.setCenter(1,1);
  c.print();

  glFlush();  // Render now
}

class MainWindow {
  int width, height;

 public:
  MainWindow() {
    width = 500;
    height = 500;
  }

  static void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D((GLdouble) 0.0, (GLdouble) w, 0.0, (GLdouble) h);
  }

  void setup(void display()) {
    glutInitWindowSize(width, height);   // Set the window's initial width & height
    glutInitWindowPosition(150, 150); // Position the window's initial top-left corner
    glutReshapeFunc(reshape);
    glutCreateWindow("Minimax GUI"); // Create a window with the given title
    glutDisplayFunc(display); // Register display callback handler for window re-paint
  }

  void run() {
    glutMainLoop();           // Enter the infinitely event-processing loop
  }
};

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {
  glutInit(&argc, argv);                 // Initialize GLUT
  //glutInitDisplayMode(GLUT_RGBA);
  MainWindow app;

  app.setup(&display);
  app.run();

  return 0;
}
