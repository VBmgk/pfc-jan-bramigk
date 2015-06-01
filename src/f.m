function y = f(x)
  [line col] = size(x);
  %y = 0;
  %for i = 1:line
  %  y += (x(i) - i) ^ 3 - exp(x(i)+i);
  %end

  %y = sin(.5 * x(1)^2 - 0.25 * x(2)^2 + 3) * cos(2*x(1) + 1 - exp(x(2)));
  %y = (1 - x(1))^2 + 100 * (x(2) - x(1)^2)^2;
  y = x(1)^3 - 12 * x(1) * x(2) + x(2)^3 - 63 * (x(1) + x(2));
endfunction
