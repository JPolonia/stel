#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "erlang.h"

#define NUM_INTERVALS	100

#define SIM_TIME    10000

#define LAMBDA  8.0
#define DELTA   1 / (LAMBDA * 1000)

int main(int argc, const char *argv[]) {

	unsigned num_intervals = NUM_INTERVALS;
	double mean = 0.0;
	
	int fd = open("counters", O_WRONLY|O_CREAT|O_TRUNC);
	if (fd < 0) {
		perror("error opening file");
		exit(0);
	}
	
	double sim_time = SIM_TIME;

	if (argc > 1)
		sscanf(argv[1], "%lf", &sim_time);
	
	unsigned long *counters = calloc(num_intervals, sizeof(*counters));
	if (!counters) {
		printf("Unable to allocate memory for counters\n");
		return 0;
	}
	
	double max_expected = erlang_max(1, LAMBDA);
	char line[30];
	sprintf(line, "%lf\n%d\n\n", max_expected, num_intervals);
	write(fd, line, strlen(line));

	srand(time(NULL));

	double curr_time = 0, prev = 0;
	int n_events = 0;
	for (; curr_time < sim_time;) {
			curr_time += DELTA;
			double c = (double)(rand()+1.0) / (RAND_MAX+1.0);
			if (c > DELTA * LAMBDA)
				continue;
		
			int index = (int)((curr_time - prev) / max_expected * (NUM_INTERVALS - 1));
			if (index < NUM_INTERVALS - 1)
				++counters[index];
			++n_events;
			mean += curr_time - prev;
			prev = curr_time;
		}
		mean /= n_events;

		printf("%d calls simulated\n", n_events);

	for (int i = 0; i < NUM_INTERVALS; i++) {
		sprintf(line, "%lu\n", counters[i]);
		write(fd, line, strlen(line));
		
	}
		
	printf("mean value: %lf\n", mean);

	return 0;
}
