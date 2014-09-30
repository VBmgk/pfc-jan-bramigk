class MyOpenGl{
  int width, height;

public:
  MyOpenGl(){
    width = 500;
    height = 500;
  }

  static void reshape(int w, int h){
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D((GLdouble) 0.0, (GLdouble) w, 0.0, (GLdouble) h);
  }

  void setup(void display()){
    glutInitWindowSize(width, height);   // Set the window's initial width & height
    glutInitWindowPosition(150, 150); // Position the window's initial top-left corner
    glutReshapeFunc(reshape);
    glutCreateWindow("Minimax Eval Function Test"); // Create a window with the given title
    glutDisplayFunc(display); // Register display callback handler for window re-paint
  }

  void run(){
    glutMainLoop();           // Enter the infinitely event-processing loop
  }
};
