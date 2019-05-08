#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "linked_list.h"
#include "poisson.h"
#include "erlang.h"

#define ulong unsigned long

#define MAX_EVENTS (100000L)
#define HIST_INTERVALS (200)

#define LAMBDA		((double)80/3600)
#define AS_CALLS	((double)0.7)

#define BUFFER_SIZE		(32)
#define AS_BUFFER_SIZE	(0)

#define GP_MIN_TIME	((double)1*60)
#define GP_MAX_TIME	((double)5*60)
#define GP_AVG_TIME	((double)2*60)
#define GP_CHANNELS	(6)

#define AS_MIN_TIME	((double)30)
#define AS_MAX_TIME	((double)120)
#define AS_AVG_TIME	((double)1*60)
#define AS_STDDEV	((double)20)

#define AS_SS_MIN	((double)1*60)
#define AS_SS_AVG	((double)2.5*60)

#define EVENT_START	0
#define EVENT_END	1
#define EVENT_AS	(1<<1)

#define error(x) do{perror(x);exit(EXIT_FAILURE);}while(0)
#define next_event(x) ((x) = list_pop(x))

int is_as(void) {
	return (double)rand()/RAND_MAX < AS_CALLS;
}

int main(int argc, const char *argv[]) {

	ulong max_events = MAX_EVENTS;
	if (argc > 1)
		sscanf(argv[1], "%lu", &max_events);

	srand(time(NULL));

	ulong *counters = calloc(HIST_INTERVALS, sizeof(*counters));
	if (!counters)
		error("calloc");

	const char *filename = "counters";
	FILE *f = fopen(filename, "w");
	if (!f)
		error("fopen");

	//fprintf(f, "%lf\n%lf\n%d\n\n", WAIT_TIME_MIN, WAIT_TIME_MAX, HIST_INTERVALS);

	list *events = NULL;

	list *gp_buffer = NULL;
	list *as_buffer = NULL;

	ulong total_events = 0, as_events = 0;
	ulong gp_active = 0, as_active = 0;
	ulong gp_waiting = 0, as_waiting = 0;
	ulong delayed = 0, as_delayed = 0;
	ulong blocked_users = 0; // AS events can't block as queue is infinite

	events = list_add(events, 0.0L, EVENT_START);
	++total_events;

	for (double sim_time; total_events < max_events; next_event(events)) {
		sim_time = events->time;
		//printf("%.2lf\t", sim_time);
		switch (events->type & EVENT_END) {
			case EVENT_START:
				//printf("EVENT_START - ");
				;double dur = erlang_random(1, LAMBDA);
				events = list_add(events, sim_time + dur, EVENT_START);
				//printf("added next start: %.2lf - ", sim_time + dur);
				++total_events;


				if (gp_active < GP_CHANNELS) {
					double dur = GP_MIN_TIME + erlang_random(1, GP_AVG_TIME);
					//printf("cur call end: %.2lf\n", sim_time + dur);
					events = list_add(events, sim_time + (dur > GP_MAX_TIME? GP_MAX_TIME: dur), EVENT_END);
					++gp_active;
				} else {
					//printf("all channels are busy.\n\t\t");
					if (gp_waiting < BUFFER_SIZE && BUFFER_SIZE > 0) {
						//printf("User queued\n");
						gp_buffer = list_add(gp_buffer, sim_time, EVENT_START);
						++gp_waiting;
						++delayed;
					} else {
						//printf("Queue is full. User dropped (%lu)\n", blocked_users);
						++blocked_users;
					}
				}
				break;
			case EVENT_END:
				if (gp_waiting > 0) {
					double waited = sim_time - gp_buffer->time;
					next_event(gp_buffer);
					--gp_waiting;
					double dur = GP_MIN_TIME + erlang_random(1, GP_AVG_TIME);
					events = list_add(events, sim_time + (dur > GP_MAX_TIME? GP_MAX_TIME: dur), EVENT_END);
				} else {
					--gp_active;
				}
				break;
			case EVENT_AS | EVENT_START:
			case EVENT_AS | EVENT_END:;
		}
	}

	printf("total users: %lu (%.2lf%% as events)\n", total_events, (double)as_events*100/total_events);
	printf("gp delayed users: %lu (%.2lf%%)\n", delayed, (double)delayed*100/total_events);
	printf("blocked users: %lu (%.2lf%%)\n", blocked_users, (double)blocked_users*100/total_events);
	//printf("inem delayed users: %lu (%.2lf%%)\n", inem_delayed_users, (double)inem_delayed_users*100/emerg_events);
	//printf("avg delay: %.2lf seconds\n", (double)(pc_total_delay + inem_total_delay)/(pc_delayed_users + inem_delayed_users));
	//printf("avg delay: pc - %.2lfs, inem - %.2lfs\n", (double)pc_total_delay/pc_delayed_users, (double)inem_total_delay/inem_delayed_users);
	//printf("inem avg delay: %.2lfs\n", (double)inem_pc_delay/emerg_events);
	//printf("waiting time error: %.2lfs (%.2lf%%)\n", waiting_time_error, (double)waiting_time_error*100/pc_total_delay);
	return 0;
}
