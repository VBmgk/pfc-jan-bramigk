class Circle{
  GLfloat center_x, center_y, radius;
  int N_MAX_POINTS;

public:
  Circle(void){
    N_MAX_POINTS = 30;

    center_x = 0.0f;
    center_y = 0.0f;

    radius = 0.5f;
  }

  void setRadius(float r){
    radius = r;
  }

  void setCenter(float x, float y){
    center_x = x;
    center_y = y;
  }

  void print(){
    glBegin(GL_POLYGON);
      glColor3f(1.0f, 0.0f, 0.0f);
      glVertex2f( center_x, center_y);
      for(int i=0; i<=N_MAX_POINTS ;i++)
        glVertex2f( center_x + radius * cos(2.0 * M_PI * i / N_MAX_POINTS), center_y + radius * sin(2.0 * M_PI * i / N_MAX_POINTS));
    glEnd();
  }
};
