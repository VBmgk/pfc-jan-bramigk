class Vector{
  arma::vec a_v;
  static const unsigned int VEC_SIZE = 2;

  Vector(const arma::vec &v): a_v(v){}

public:
  Vector(){
    a_v = arma::zeros<arma::vec>(Vector::VEC_SIZE);
  }

  static Vector getURand(){
    Vector aux;
    arma::arma_rng::set_seed_random();

    aux.a_v = arma::randu<arma::vec>(Vector::VEC_SIZE);

    return aux;
  }

  static Vector getNRand(const Vector &v){
    Vector aux;
    arma::arma_rng::set_seed_random();

    // Center Distribution on last value
    aux.a_v = v.a_v + arma::randn<arma::vec>(Vector::VEC_SIZE);

    return aux;
  }

  float norm(){
    return (float) arma::norm(a_v);
  }

  Vector operator+(const Vector& v2) const {
    Vector vr(a_v + v2.a_v);
    return vr;
  }

  Vector operator-(const Vector& v2) const {
    Vector vr(a_v - v2.a_v);
    return vr;
  }

  float operator*(const Vector& v2) const {
    //return a_v.t() * v2.a_v;
    return 0.0;
  }
};
