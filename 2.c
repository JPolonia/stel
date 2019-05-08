#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "linked_list.h"
#include "erlang.h"

#define LAMBDA				((double)12 / 3600)
#define AVG_SERVICE_TIME	((double)(2 * 60))
#define N_CHANNELS			8
#define BUFFER_SIZE			4
#define POP_SIZE			20
#define SIM_TIME			36000

#define HIST_INTERVALS		100

#define START				0
#define END					1

double avg_waiting_time(double lambda, double avg_service_time, int m);

int main(int argc, const char *argv[]) {
	
	double sim_time = SIM_TIME;
	if (argc > 1)
		sscanf(argv[1], "%lf", &sim_time);

	const char *filename = argc < 3? "counters": argv[2];

	unsigned long *counters = calloc(HIST_INTERVALS, sizeof(*counters));
	if (!counters) {
		perror("calloc");
		return 0;
	}

	double max_expected = 25.0 * 10;
	double max_delay = 0.0;
	
	FILE *f = fopen(filename, "w");
	if (!f) {
		perror("fopen");
		return 0;
	}

	fprintf(f, "%lf\n%d\n\n", max_expected, HIST_INTERVALS);

	int active_users = 0;
	int waiting_users = 0;

	unsigned long n_events = 0;
	unsigned long serviced_users = 0;
	unsigned long delayed_users = 0;
	unsigned long blocked_users = 0;

	double waiting_time = 0.0;

	list *lst = NULL;
	list *buffer = NULL;

	double first_event_start = erlang_random(1, (POP_SIZE - active_users) * LAMBDA);
	lst = list_add(lst, first_event_start, START);
	if (!lst)
			return 0;

	for (double time = 0.0; time < sim_time; lst = list_pop(lst)) {
		time = lst->time;

		if (lst->type == END) {
			if (!waiting_users) {
				--active_users;
				continue;
			}
			waiting_time += time - buffer->time;
			max_delay = time - buffer->time > max_delay? time - buffer->time: max_delay;
			int index = (int)((time - buffer->time) / max_expected * (HIST_INTERVALS - 1));
			if (index < HIST_INTERVALS - 1)
				++counters[index];
			++serviced_users;
			double event_duration = erlang_random(1, 1/AVG_SERVICE_TIME);
			lst = list_add(lst, time + event_duration, END);
			if (!lst)
				return 0;
			buffer = list_pop(buffer);
			--waiting_users;
			continue;
		}
		++n_events;
		double next_event_start = erlang_random(1, (POP_SIZE - active_users - waiting_users) * LAMBDA);
		lst = list_add(lst, time + next_event_start, START);
		if (!lst)
				return 0;
		if (active_users < N_CHANNELS) {
			++active_users;
			double event_duration = erlang_random(1, 1/AVG_SERVICE_TIME);
			lst = list_add(lst, time + event_duration, END);
			if (!lst)
				return 0;
			++serviced_users;
			continue;
		}
		if (waiting_users < BUFFER_SIZE) {
			buffer = list_add(buffer, time, START);
			++waiting_users;
			++delayed_users;
			continue;
		}
		++blocked_users;
	}

	for (int i = 0; i < HIST_INTERVALS; ++i)
		fprintf(f, "%lu\n", counters[i]);

	fclose(f);

	printf("Simulated %lf seconds - %lu events generated\n", sim_time, n_events);
	printf("Blocked users: %lu\t\tP(blocked) = %lf\n", blocked_users, (double)blocked_users/n_events);
	printf("Delayed users: %lu\t\tP(delayed) = %lf\n", delayed_users, (double)delayed_users/serviced_users);
	printf("Average costumer service delay = %lf seconds\n", delayed_users == 0? 0: (waiting_time/delayed_users)*(double)delayed_users/serviced_users);
	printf("Maximum delay: %lf\n", max_delay);

	return 0;
}

