class Vector{
  float x,y, theta;

public:
  Vector():
    x(0), y(0), theta(0){}

  Vector(float x, float y):
    x(x), y(y), theta(0){}

  Vector(float x, float y, float theta):
    x(x), y(y), theta(theta){}

  float getX(){
    return x;
  }

  float getY(){
    return y;
  }

  float getTheta(){
    return theta;
  }

  float norm(){
    return sqrt(x*x + y*y);
  }

  Vector operator + (const Vector& v){
    Vector r(this->x + v.x, this->y + v.y);

    return r;
  }

  float operator * (const Vector& v){
    return this->x * v.x + this->y * v.y;
  }
};
