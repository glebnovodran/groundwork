n = 8;
A = zeros(n);
for i = 1:n
	for j = 1:n
		A(i,j) = factorial(i + j - 2) / ( factorial(i - 1) *factorial(j - 1) );
	end
end