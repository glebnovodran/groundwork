function R = rodrigues(w, t)
	wn = norm(w);
	th = deg2rad(t)*wn;
	ax = w / wn;
	wh = [0, -ax(3), ax(2); ax(3), 0, -ax(1); -ax(2), ax(1), 0];
	R = eye(3) + wh*sin(th) + (wh^2)*(1 - cos(th));
end
