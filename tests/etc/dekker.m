n = 4;
A = zeros(n);
for i = 1:n
	for j = 1:n
		A(i,j) = factorial(n + i - 1) / ( factorial(n - j) * factorial(i - 1) * factorial(j - 1) * (i + j - 1) );
	end
end
