% Computation of Hessian matrix
function [H, eig_min, eig_max] = hessian(x, TOL)
  N = size(x)(1);

  % Hessian matrix
  H = zeros(N, N);

  for i = 1:N
    x_n     =  x;
    x_n(i) += -TOL;

    x_p     =  x;
    x_p(i) += +TOL;

    % Hessian
    % H(:,1)'= del  f1        del  fn
    %         -----   , ..., -----
    %         del x1         del x1
    H(:,i) = (grad_f(x_p, TOL) - grad_f(x_n, TOL))' * (1.0/(2 * TOL));
  end

  % sorting eigen values
  eig_values = sort(eig(H));
  
  % get minimum and maximun eigen value
  eig_min = eig_values(1);
  eig_max = eig_values(N);
end
