#define DEFAUL_SPEED 10 // speed iinm/s

class Kick: public Action{
  float speed, angle;
public:
  Kick(class Robot);
};
