#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "linked_list.h"
#include "poisson.h"
#include "erlang.h"

#define LAMBDA				((double)600 / 3600)

#define START				0
#define END					1
#define EMERG				2
#define INEM				4

#define PC_START			START
#define PC_END				END
#define EMERG_END			(EMERG | END)
#define INEM_END			(INEM | END)

#define PC_EVENT_PROB		((double)0.4)
#define PC_MIN_SERV_TIME	((double)1 * 60)
#define PC_MAX_SERV_TIME	((double)4 * 60)
#define PC_AVG_SERV_TIME	((double)1.5 * 60)
#define PC_BUFFER_SIZE		(8)
#define PC_N_CHANNELS		(16)

#define EMERG_EVENT_PROB	((double)0.6)
#define EMERG_MIN_SERV_TIME	((double)30)
#define EMERG_MAX_SERV_TIME	((double)75)
#define EMERG_AVG_SERV_TIME	((double)45)
#define EMERG_SD_SERV_TIME	((double)15)

#define INEM_MIN_SERV_TIME	((double)1 * 60)
#define INEM_AVG_SERV_TIME	((double)1.5 * 60)
#define INEM_N_CHANNELS		(8)

#define SIM_TIME			(36000)
#define MAX_EVENTS			(100000)

#define WAIT_TIME_MIN		((double)-200)
#define WAIT_TIME_MAX		((double)200)
#define HIST_INTERVALS		((int)(WAIT_TIME_MAX - WAIT_TIME_MIN))

#define error(x)			do{perror(x);exit(EXIT_FAILURE);}while(0)
#define next_event(x)		((x) = list_pop(x))

bool nprob(double p);
list *add_event_end(list *events, double time, bool emerg);

int main(int argc, const char *argv[]) {
	
	double sim_time = SIM_TIME;
	unsigned long max_events = MAX_EVENTS;

	if (argc > 1)
		//sscanf(argv[1], "%lf", &sim_time);
		sscanf(argv[1], "%lu", &max_events);

	srand(time(NULL));

	const char *filename = argc < 3? "counters": argv[2];

	unsigned long *counters = calloc(HIST_INTERVALS, sizeof(*counters));
	if (!counters)
		error("calloc");

	FILE *f = fopen(filename, "w");
	if (!f)
		error("fopen");
	
	fprintf(f, "%lf\n%lf\n%d\n\n", WAIT_TIME_MIN, WAIT_TIME_MAX, HIST_INTERVALS);

	list *events = NULL;

	list *pc_buffer = NULL;
	list *inem_buffer = NULL;

	double start = 0.0L;
	events = list_add(events, start, 0, START);
	if (!events)
		error("list_add");

	int pc_active_users = 0;
	int pc_buffered_users = 0;
	int inem_active_users = 0;
	unsigned long inem_buffered_users = 0;
	unsigned long blocked_users = 0;
	unsigned long pc_delayed_users = 0;
	unsigned long inem_delayed_users = 0;
	unsigned long total_users = 1;
	unsigned long emerg_events = 0;
	double pc_total_delay = 0.0L;
	double inem_total_delay = 0.0L;
	double inem_pc_delay = 0.0L;
	double waiting_time_error = 0.0L;

	for (double simt = start; total_users < max_events; next_event(events), simt = events->time) {
	//for (double simt = start; simt < sim_time; next_event(events), simt = events->time) {
		switch (events->type) {
			case PC_START:
				events = list_add(events, simt, erlang_random(1, LAMBDA), PC_START);
				++total_users;
				if (pc_active_users < PC_N_CHANNELS) {
					++pc_active_users;
					bool emerg = nprob(EMERG_EVENT_PROB);
					if (emerg)
						++emerg_events;
					events = add_event_end(events, simt, emerg);
					break;
				}
				if (pc_buffered_users < PC_BUFFER_SIZE) {
					++pc_delayed_users; ++pc_buffered_users;
					pc_buffer = list_add(pc_buffer, simt, 0, PC_START);
					break;
				}
				++blocked_users;
				break;
			case PC_END:
				if (pc_buffered_users > 0) {
					--pc_buffered_users;
					pc_total_delay += simt - pc_buffer->time;
					double wait_error = ((double)pc_total_delay / pc_delayed_users) - (simt - pc_buffer->time);
					double wait_err_rel = wait_error * 100 / ((double)pc_total_delay / pc_delayed_users);
					next_event(pc_buffer);
					bool emerg = nprob(EMERG_EVENT_PROB);
					if (emerg)
						++emerg_events;
					events = add_event_end(events, simt, emerg);
					int index = (int)ceil(wait_err_rel - WAIT_TIME_MIN);
					if (index < HIST_INTERVALS && index >= 0)
						++counters[index];
					printf("index: %d\n", index);
					waiting_time_error += wait_error;
					break;
				}
				--pc_active_users;
				break;
			case EMERG_END:
				if (inem_active_users < INEM_N_CHANNELS) {
					inem_pc_delay += simt - events->arrival_time;
					++inem_active_users;
					--pc_active_users;
					double dur = INEM_MIN_SERV_TIME + erlang_random(1, 1/INEM_AVG_SERV_TIME);
					events = list_add(events, simt, dur, INEM_END);
					break;
				}
				++inem_buffered_users;
				++inem_delayed_users;
				inem_buffer = list_add(inem_buffer, events->arrival_time, simt - events->arrival_time, EMERG_END);
				break;
			case INEM_END:
				if (inem_buffered_users > 0) {
					--inem_buffered_users;
					inem_total_delay += simt - inem_buffer->time;
					inem_pc_delay += simt - inem_buffer->arrival_time;
					next_event(inem_buffer);
					--pc_active_users;
					double dur = INEM_MIN_SERV_TIME + erlang_random(1, 1/INEM_AVG_SERV_TIME);
					events = list_add(events, simt, dur, INEM_END);
				}
				--inem_active_users;
				break;
			default:
				printf("Unknown event type\n");
		}
	}

	for (int i = 0; i < HIST_INTERVALS; ++i)
		fprintf(f, "%lu\n", counters[i]);

	fclose(f);

	printf("total users: %lu (%.2lf%% emerg)\n", total_users, (double)emerg_events*100/total_users);
	printf("pc delayed users: %lu (%.2lf%%)\n", pc_delayed_users, (double)pc_delayed_users*100/total_users);
	printf("blocked users: %lu (%.2lf%%)\n", blocked_users, (double)blocked_users*100/total_users);
	printf("inem delayed users: %lu (%.2lf%%)\n", inem_delayed_users, (double)inem_delayed_users*100/emerg_events);
	printf("avg delay: %.2lf seconds\n", (double)(pc_total_delay + inem_total_delay)/(pc_delayed_users + inem_delayed_users));
	printf("avg delay: pc - %.2lfs, inem - %.2lfs\n", (double)pc_total_delay/pc_delayed_users, (double)inem_total_delay/inem_delayed_users);
	printf("inem avg delay: %.2lfs\n", (double)inem_pc_delay/emerg_events);
	printf("waiting time error: %.2lfs (%.2lf%%)\n", waiting_time_error, (double)waiting_time_error*100/pc_total_delay);
	return 0;
}

bool nprob(double p) {
	return ((double)(rand() % RAND_MAX + 1) / ((double)RAND_MAX + 1)) < p;
}

list *add_event_end(list *events, double time, bool emerg) {
	double dur = 0.0L;
	if (!emerg)
		do
			dur = PC_MIN_SERV_TIME + erlang_random(1, 1/PC_AVG_SERV_TIME);
		while (dur > PC_MAX_SERV_TIME);
	else
		do
			dur = poisson_random(EMERG_AVG_SERV_TIME, EMERG_SD_SERV_TIME);
		while (dur < EMERG_MIN_SERV_TIME || dur > EMERG_MAX_SERV_TIME);
	return list_add(events, time, dur, emerg? EMERG_END: PC_END);
}
