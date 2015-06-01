% Find local minimal or maximal point using newton method and
% gradient method
%
% References
%   "Métodos de Cãlculo Numérico", by José Paulo P. Dieguez
%   "Calculus Volume II", by Tom M. Apostol

function [x_k, erro] = newton_grad(x_0, gamma, MAX_IT, TOL, extr_type)
  MAX_SUB_IT = MAX_IT * 1;
  MAX_VALUE = 3000;
  % number of lines of x_0
  N = size(x_0)(1);

  % initial values
  x_k = x_0; erro = 1; n_it = 1; n_sub_it = 1;

  % Main iteration loop
  while (n_it < MAX_IT && erro > TOL)
    printf("main loop\n"); n_sub_it = 1; n_it += 1;

    [H, eig_min, eig_max] = hessian  (x_k, TOL);

    % Discover the type of stationary point.
    % All eigen values must have the same signal on an extremal point
    % (if some conditions are satisfied, vide apostol volume II)
    % minimum: all eigen values must be positive
    if(strcmp(extr_type,"min")) printf("minimum\n");
      % change direction until hessian eigen values are positive
      while((eig_min < 0 || eig_min * eig_max < 0) && n_sub_it < MAX_SUB_IT)
        n_sub_it += 1;
        [x_k, erro]           = grad_min (x_k, -gamma, MAX_IT, TOL);
        [H, eig_min, eig_max] = hessian  (x_k, TOL);

        printf("x_k: %f %f, gamma: %f, eig_min: %f, eig_max: %f\n",x_k(1), x_k(2), -gamma, eig_min, eig_max);
      endwhile
      % end if the eigen values did not change
      if(n_sub_it == MAX_SUB_IT) printf("barro\n"); n_it = MAX_IT; endif
    endif

    % maximum: all eigen values must be negative
    if(strcmp(extr_type,"max"))
      printf("maximum\n");
      % change direction until hessian eigen values are negative
      while ((eig_min > 0 || eig_min * eig_max < 0) && n_sub_it < MAX_SUB_IT)
        n_sub_it += 1;
        [x_k, erro]           = grad_min (x_k,  gamma, MAX_IT, TOL);
        [H, eig_min, eig_max] = hessian  (x_k, TOL);
      endwhile
      % end if the eigen values did not change
      if(n_sub_it == MAX_SUB_IT) n_it = MAX_IT; endif
    endif

    % this is done after to avoid inflexion points, because things
    % may go to space if we go to this points
    [x_k, erro, eig_min, eig_max] = newton_min(x_k, MAX_IT/2, TOL);
    printf("eig min: %f, product: %f\n", eig_min, eig_min * eig_max);

    % NOTE: If the Hessian is not symetric, then some eigen values
    % can be complex. But this is not verified here
  endwhile
endfunction
