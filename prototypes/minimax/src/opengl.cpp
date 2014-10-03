#include <iostream>
#include <vector>
#include <cmath>
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/OpenGL.h>
#else
#include <GL/glut.h>
#endif

#include "graphical_classes.h"
#include "my_opengl.h"

using namespace std;

class Field{
  GLfloat w,h;
  GLfloat goal_size, goal_w;
public:
  Field(){
    w = 5.4f;
    h = 7.4f;

    goal_size = 0.7f;
    goal_w = 0.3f;
  }

  void print(){
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

/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {
  glutInit(&argc, argv);                 // Initialize GLUT
  MyOpenGl opengl;

  opengl.setup(&display);
  opengl.run();

  return 0;
}
