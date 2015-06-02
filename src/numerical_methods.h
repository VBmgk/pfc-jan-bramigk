/**********************************************************
 * References:                                            *
 *  - "Métodos de Cãlculo Numérico",                      *
 *            by professor José Paulo P. Dieguez          *
 *  - "Calculus Volume II",                               *
 *            by Tom M. Apostol                           *
 **********************************************************/
#ifndef MOVE_SOLVER_H
#define MOVE_SOLVER_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <tuple>
#include <array>
#include <algorithm>

#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <mat2cpp.hpp>
#include "lapacke.h"

namespace numerical_method {

using namespace boost::numeric::ublas;
using namespace mat2cpp;
using namespace std;
using num = double;
using mat = matrix<num>;
using zeros = zero_matrix<num>;
using iden = identity_matrix<num>;
using vec = boost::numeric::ublas::vector<num>;

template <typename T> num norm(const T e) {
  return max(abs(max(e)), abs(min(e)));
}
num norm(const vec e) { return sqrt(inner_prod(e, e)); }

// N_MAX: maximum value above which computer freezes
#define N_MAX 2000
#define DELTA 0.0001
#define MAX_VALUE 3000
#define MAX_IT 100
#define TOL 0.0001
#define MAX_SUB_IT MAX_IT

/************
 * Gradiend *
 ************/
vec grad(num (*f)(const vec), const vec x) {
  vec grad(x.size());
  for (unsigned int i = 0; i < x.size(); i++) {
    vec x_pls_delta = x, x_mns_delta = x;
    // using central diferences
    x_pls_delta(i) += DELTA;
    x_mns_delta(i) += -DELTA;
    grad(i) = (f(x_pls_delta) - f(x_mns_delta)) / (2 * DELTA);
  }
  return grad;
}

/**********************************************************
 * Method to find extremal using the gradiend descendent  *
 * (gamma < 0) or ascendent (gamma > 0) method, depending *
 * on the sinal of gamma.                                 *
 **********************************************************/
tuple<vec, num> grad_min(num (*f)(const vec), const vec x_init,
                         const num gamma) {
  vec x_k(x_init.size()), x_buff(x_init.size());
  unsigned int n_it = 1;
  num erro = 1;
  // setup initial value of x_k
  x_k = x_init;
  // main loop
  while (n_it < MAX_IT && (erro > TOL)) {
    n_it = 1 + n_it;
    // main gradient method equation
    x_buff = x_k + gamma * grad(f, x_k);
    // check if something went wrong and go to zero
    if (norm(x_buff) > MAX_VALUE) {
      x_buff *= 0.0;
    }
    // compute the error in x and update x_k
    x_buff -= x_k;
    erro = norm(x_buff);
    x_k += x_buff;
  }

  return make_tuple(x_k, erro);
}

/*********************************
 * Computation of Hessian matrix *
 *********************************/
int eig(const mat &_A, num *eigen_values) {
  int info = 0, n = _A.size1(), order_of_A = _A.size1(),
      size_work_array = _A.size1() * 42;
  num *A = nullptr, *work_array = nullptr;
  // dont calculate eigenvectors
  char return_eigen_vectors = 'N', upper_lower_triag = 'U';
  // check if matrix _A is square
  if (_A.size1() == _A.size2()) {
    // allocate memory
    A = new num[n * n], work_array = new num[size_work_array];
    // free memory
    copy(_A.data().begin(), _A.data().end(), A);
    // call lapack subrotine
    // XXX: This only works with double!!!
    LAPACK_dsyev(&return_eigen_vectors, &upper_lower_triag, &order_of_A, A, &n,
                 eigen_values, work_array, &size_work_array, &info);
  } else {
    info = -1;
  }
  // free memory
  delete A;
  delete work_array;

  return info;
}
// compute greatest and lowest eigenvalues
tuple<num, num, int> max_min_eigs(const mat &A) {
  num *eigenvalues = new num[A.size1()], max = 0, min = 0;
  int info = -1;
  // check for maximum size
  if (A.size1() < N_MAX) {
    // compute eigenvalues
    info = eig(A, eigenvalues);
    // lowest eigen value
    min = eigenvalues[0];
    // greatest eigen value
    max = eigenvalues[A.size1() - 1];
  }
  // free memory
  delete eigenvalues;
  return make_tuple(max, min, info);
}
// compute hessian and its greatest and lowest eigenvalues
tuple<mat, num, num, int> hessian(num (*f)(const vec), const vec x) {
  // Hessian matrix
  mat H = zeros(x.size(), x.size());
  num eig_min, eig_max;
  vec x_n(x.size()), x_p(x.size());
  int info = -1;

  for (unsigned i = 0; i < x.size(); i++) {
    x_n = x;
    x_n(i) += -TOL;
    x_p = x;
    x_p(i) += +TOL;
    // Hessian
    // H(:,1)'= del  f1        del  fn
    //         -----   , ..., -----
    //         del x1         del x1
    row(H, i) = (grad(f, x_p) - grad(f, x_n)) * (1.0 / (2 * TOL));
  }
  // Note that we changed the rows for the columns, so it would
  // be good to transpose H, but it should be simetric... but
  // for the sake of readability, it is here: H = trans(H);

  // get minimum and maximun eigen value
  tie(eig_max, eig_min, info) = max_min_eigs(H);

  return make_tuple(H, eig_min, eig_max, info);
}

/**********************************************************
 * Find inflexion point using newton method               *
 **********************************************************/
tuple<vec,num,num,num,int> newton_min(num (*f)(const vec), const vec x) {
  unsigned int N = x.size(), n_it = 1;
  int info = -1;
  num erro = 1, eig_min, eig_max;
  mat H(N, N), _grad(N, 1);
  // Initial value for x
  vec x_k(N), z(N);
  x_k = x;
  // Main iteration loop
  while (n_it < MAX_IT && erro > TOL) {
    n_it += 1;
    // Compute hessian matrix
    // One need to analize the eigen values to find the type of
    // inflexion point
    tie(H, eig_min, eig_max, info) = hessian(f, x_k);
    // Solve H.z = -grad(f), (on octave: z = H \ - grad_f(x_k, TOL))
    // transform vector to matrix, using z as a buffer here
    size_t _r;
    z = -grad(f, x_k);
    copy(z.begin(), z.end(), _grad.begin1());
    z = column(matrix_div(H, _grad, _r), 0);
    // increment size
    x_k += z;
    erro = norm(z);
  }
  // TODO: pass info in a better way
  return make_tuple(x_k, erro, eig_min, eig_max, info);
}

/***************************************************************
 * Find local minimal or maximal point using newton method and *
 * gradient method                                             *
 ***************************************************************/
tuple<vec, num> newton_grad(num (*f)(const vec), const vec x_0, const num gamma,
                            const bool find_minimum) {
  // initial values
  unsigned int n_it = 1, n_sub_it = 1, n = x_0.size();
  int info = -1;
  num erro = 1, eig_min, eig_max;
  vec x_k(n);
  x_k = x_0;
  mat H(n, n);
  // Main iteration loop
  while (n_it < MAX_IT && erro > TOL) {
    n_sub_it = 1;
    n_it += 1;
    // First, get out of inflexion points, because the gradient may
    // increase
    // fast in that points. For this to be done, it is necessary to
    // discover
    // the type of stationary point.
    tie(H, eig_min, eig_max, info) = hessian(f, x_k);
    // All eigen values must have the same signal on an extremal point
    // (if some conditions are satisfied, vide apostol volume II)
    if (find_minimum) {
      // MINIMUM: all eigen values must be positive
      // change direction until hessian eigen values are positive
      while ((eig_min < 0 || eig_min * eig_max < 0) && n_sub_it < MAX_SUB_IT) {
        n_sub_it += 1;
        tie(x_k, erro) = grad_min(f, x_k, -gamma);
        tie(H, eig_min, eig_max, info) = hessian(f, x_k);
      }
    } else {
      // MAXIMUM: all eigen values must be negative
      // change direction until hessian eigen values are negative
      while ((eig_min > 0 || eig_min * eig_max < 0) && n_sub_it < MAX_SUB_IT) {
        n_sub_it += 1;
        tie(x_k, erro) = grad_min(f, x_k, gamma);
        tie(H, eig_min, eig_max, info) = hessian(f, x_k);
      }
    }
    // end main if the eigen values did not change
    if (n_sub_it == MAX_SUB_IT) {
      break;
    }
    // this is done after to avoid inflexion points, because things
    // may go to space if we go to this points
    tie(x_k, erro, eig_min, eig_max, info) = newton_min(f, x_k);
    // NOTE: If the Hessian is not symetric, then some eigen values
    // can be complex. But this is not verified here
  }
  return make_tuple(x_k, erro);
}
// undefining local defines
#undef DELTA
#undef MAX_VALUE
#undef MAX_IT
#undef TOL
#undef N_MAX
#undef MAX_SUB_IT
}

#endif
