#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "erlang.h"

double nrandom(unsigned n) {
	double a = 1;
	for (int i = 0; i < n; i++)
		a *= (double)(rand() % RAND_MAX + 1) / ((double)RAND_MAX + 1);
	return a;
}

double erlang_random(unsigned k, double rate) {
	return -1/rate * log(nrandom(k));
}

double erlang_max(unsigned k, double rate) {
	return (double)k/rate + 5*sqrt(k)/rate;
}

