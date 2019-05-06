/*
#include <cstdio>
#include <iostream>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

void rgb2lab(const Mat _image)
{
	//function y = rgb2lab(im)
	//	% rgb2lab(image)
	//	% Converte de RGB para LAB
	//	% Define as constantes
	float Xn = 95.13;
	float Yn = 100.0;
	float Zn = 108.86;
	//disp('Iniciando a conversao de RGB para LAB');
	// Converte de RGB para XYZ
	Mat	xyz = rgb2xyz(_image);
	bx = xyz(:, : , 1);
	by = xyz(:, : , 2);
	bz = xyz(:, : , 3);

	% Calcula X / Xn, Y / Yn e Z / Zn
		bx = bx / Xn;
	by = by / Yn;
	bz = bz / Zn;

	% Determina os elem.maiores ou iguais a 8.856E-3
		bin = (bx >= 8.856E-3);
	f_x = (bin.*bx) . ^ (1 / 3);
	bin = (by >= 8.856E-3);
	yyn = (bin.*by) . ^ (1 / 3);
	f_y = yyn;
	bin = (bz >= 8.856E-3);
	f_z = (bin.*bz) . ^ (1 / 3);

	l = (116 * yyn) - 16;

	% Determina os elem.menores que 8.856E-3
		bin = (bx < 8.856E-3);
	f_x = f_x + (7.787 * (bin.*bx) + (16 / 116));
	bin = (by < 8.856E-3);
	yyn = (bin.*by);
	f_y = f_y + (7.787 * yyn + (16 / 116));
	bin = (bz < 8.856E-3);
	f_z = f_z + (7.787 * (bin.*bz) + (16 / 116));


	% Calcula L, A e B
		l = l + (903.3 * (yyn . ^ (1 / 3)));
	a = (500 * l) .* (f_x - f_y);
	b = (200 * l) .* (f_y - f_z);

	y(:, : , 1) = l;
	y(:, : , 2) = a;
	y(:, : , 3) = b;
}
*/