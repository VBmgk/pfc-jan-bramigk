function grad = grad_f(x, DELTA)
  % x is a colum vector
  line_num = size(x)(1);
  grad = zeros(line_num,1);

  for i = 1:line_num
    x_plus_delta      = x;
    x_plus_delta(i)  += +DELTA;

    x_minus_delta     = x;
    x_minus_delta(i) += -DELTA;

    grad(i,1) = (f(x_plus_delta) - f(x_minus_delta)) / (2 * DELTA);
  end
endfunction
