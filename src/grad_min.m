function [x_solution, erro, value, n_it] = grad_min(x_init, gamma, MAX_IT, TOL)
  % number of lines of x_0
  N = size(x_init)(1);
  MAX_VALUE = 3000;
  x_solution = x_init;
  n_it = 1;
  erro = 1;

  while (n_it < MAX_IT && (erro > TOL))
    n_it = 1 + n_it;

    x_buff = x_solution + gamma * grad_f(x_solution, TOL);

    % check if something went wrong and go to zero
    if (norm(x_buff) > MAX_VALUE) x_buff = zeros(N,1); endif

    erro = norm(x_buff - x_solution);
    x_solution = x_buff;
  end

  value = f(x_solution);
endfunction
