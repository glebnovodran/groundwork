N = 4;
A = zeros(N);
for i = 1:N
	for j = 1:N
		A(i,j) = sqrt((i-1)^2 + (j-1)^2);
	end
end
