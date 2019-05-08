#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>
#include <time.h>
#include <math.h>

#include "poisson.h"

double poisson_random(double mu, double sigma)
{
	static const double epsilon = DBL_MIN;
	static const double two_pi = 2.0 * M_PI;

	static double z1;
	static bool generate = false;
	generate = !generate;

	if (!generate)
	   return z1 * sigma + mu;

	double u1, u2;
	do {
	   u1 = rand() * (1.0 / RAND_MAX);
	   u2 = rand() * (1.0 / RAND_MAX);
	}
	while (u1 <= epsilon);

	double z0;
	z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
	return z0 * sigma + mu;
}

double poisson_max(double mu, double sigma) {
	return 0.0L;
}

