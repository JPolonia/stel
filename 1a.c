#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "erlang.h"

#define SIM_TIME	10000

#define NUM_INTERVALS	100

#define ERLANG_SHAPE 	1
#define ERLANG_RATE	8.0

int main(int argc, const char *argv[]) {

	unsigned num_events = 0;
	double sim_time = SIM_TIME;
	unsigned erlang_shape = ERLANG_SHAPE;
	unsigned num_intervals = NUM_INTERVALS;
	double erlang_rate = ERLANG_RATE;
	double max_expected = erlang_max(ERLANG_SHAPE, ERLANG_RATE);
	double mean = 0.0;
	
	int fd = open("counters", O_WRONLY|O_CREAT|O_TRUNC, 0755);
	if (fd < 0) {
		perror("error opening file");
		exit(0);
	}

	if (argc > 1)
		sscanf(argv[1], "%lf", &sim_time);
	if (argc > 3) {
		sscanf(argv[2], "%u", &erlang_shape);
		sscanf(argv[3], "%lf", &erlang_rate);
	}
	
	unsigned long *counters = calloc(num_intervals, sizeof(*counters));
	if (!counters) {
		printf("Unable to allocate memory for counters\n");
		return 0;
	}
	
	char line[30];
	sprintf(line, "%lf\n%d\n\n", max_expected, num_intervals);
	write(fd, line, strlen(line));

	for (double curr_time = 0.0; curr_time < sim_time; ++num_events) {
		double n = erlang_random(erlang_shape, erlang_rate);
		curr_time += n;
		int index = n / max_expected * (num_intervals - 1.0);
		if (index < num_intervals - 1)
			++counters[index];
		mean += n;
	}
	mean /= num_events;

	for (int i = 0; i < NUM_INTERVALS; i++) {
		sprintf(line, "%lu\n", counters[i]);
		write(fd, line, strlen(line));
		
	}
		
	printf("mean value: %lf\n", mean);

	return 0;
}
