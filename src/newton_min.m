% Find inflexion point using newton method
%
% References
%   "Métodos de Cãlculo Numérico", by José Paulo P. Dieguez
%   "Calculus Volume II", by Tom M. Apostol

function [x_k, erro, eig_min, eig_max] = newton_min(x_0, MAX_IT, TOL)
  % number of lines of x_0
  N = size(x_0)(1);
  H = zeros(N, N);

  % initial value
  x_k = x_0;

  erro = 1;
  n_it = 1;

  % Main iteration loop
  while (n_it < MAX_IT && erro > TOL)
    n_it = 1 + n_it;

    % Compute hessian matrix
    % One need to analize the eigen values
    % to find the type of inflexion point
    [H, eig_min, eig_max] = hessian(x_k, TOL);

    % Solve H.z = -grad(f)
    z = H \ - grad_f(x_k, TOL);

    x_k += z;
    erro = abs(z);
  endwhile
endfunction
