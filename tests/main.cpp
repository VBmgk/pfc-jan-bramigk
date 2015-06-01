#include "move_solver.h"

num f(const vec x) { 
  if (x.size() < 2) exit(1);

  return pow(x(0), 3) - 12 * x(0) * x(1) + pow(x(1), 3)
         - 63 * (x(0) + x(1));
}

int main(void) {
  num gamma = 0.05, erro,
      eig_min,   eig_max;

  mat H;

  int info;
  vec x (2), x1(2), sol(2), sol_nwtn(2);
  x  <<= M_PI_2, 0;
  x1 <<=      0, 0;

  tie(sol, erro)                = grad_min (f, x, gamma);
  tie(H, eig_min, eig_max,info) = hessian  (f, x);
  cout <<  "hessian matrix: " << H          << endl;

  tie(H, eig_min, eig_max,info) = hessian  (f, x1);

  // testing matrix_div with vectors
  mat A(3,3),_b(3,1); vec b(3);
  size_t _r;
  A <<= 1, 0, 0, 0, 1, 0, 0, 0, 1;
  b <<= 2, 3, 4;
  // transform vector to matrix
  copy(b.begin(), b.end(), _b.begin1());

  cout << "x  = " << x                      << endl
       << "x1 = " << x1                     << endl
       << "grad(f, x)  = "  << grad (f, x)  << endl
       << "grad(f, x1) = "  << grad (f, x1) << endl
       << "grad_min(f, x, " << gamma << ") = "
       << sol << " with error = " << erro   << endl
       << "hessian matrix: " << H           << endl
       << "eig max: " << eig_min            << endl
       << "eig min: " << eig_max            << endl
       << "A.x = b: "<< matrix_div(A, _b, _r) << endl;

  tie(sol_nwtn, erro, eig_min, eig_max, info) = newton_min (f, x1);
  cout << "newton_min(f,x1): " << sol_nwtn  << endl
       << "erro: "    << erro               << endl
       << "eig max: " << eig_min            << endl
       << "eig min: " << eig_max            << endl;

  tie(sol_nwtn, erro) = newton_grad (f, x1, gamma, true);
  cout << "newton_grad (f,x1): " << sol_nwtn << endl
       << "erro: "    << erro                << endl;

  tie(sol_nwtn, erro) = newton_grad (f, x1, gamma, false);
  cout << "newton_grad (f,x1): " << sol_nwtn << endl
       << "erro: "    << erro                << endl;

  return 0;
}
