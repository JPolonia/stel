#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#include "linked_list.h"
#include "lista.c"
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
#define GP_AVG_TIME	((double)2*60) // Mean time of exponential distribution (PC calls)
#define GP_MAX_CHANNELS	(6)

#define AS_MIN_TIME	((double)30)
#define AS_MAX_TIME	((double)120)
#define AS_AVG_TIME	((double)1*60)
#define AS_STDDEV	((double)20)
#define AS_MAX_CHANNELS	(6)

#define AS_SS_MIN	((double)1*60)
#define AS_SS_AVG	((double)2.5*60)

#define EVENT_START	0
#define EVENT_END	1
#define EVENT_AS	(1<<1) //2

#define error(x) do{perror(x);exit(EXIT_FAILURE);}while(0)
#define next_event(x) ((x) = list_pop(x))

//extern double urng(void);

int is_as(void) {
	//return urng() < AS_CALLS;
	return ((double)rand()/RAND_MAX) < AS_CALLS;
}

double calc_distribuition(int type){
	double dur;

	switch (type){
		case EVENT_START:
			return erlang_random(1, LAMBDA);
		case EVENT_END:
			do {
    	  dur = GP_MIN_TIME + erlang_random(1, GP_AVG_TIME);
			} while ((dur < GP_MIN_TIME) || (dur > GP_MAX_TIME));
			return dur;
		case EVENT_AS | EVENT_START:
			do {
				dur = poisson_random(AS_AVG_TIME, AS_STDDEV);
			} while ((dur < AS_MIN_TIME) || (dur > AS_MAX_TIME));// Min time=30s, Max time=0.75s
			return dur;
		case EVENT_AS | EVENT_END:
			do {
				dur = poisson_random(AS_AVG_TIME, AS_STDDEV);
			} while ((dur < AS_MIN_TIME) || (dur > AS_MAX_TIME));// Min time=30s, Max time=0.75s
			return dur;
			break;
	}
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

	ulong total_events = 0, as_events = 0, gp_events = 0;
	ulong gp_channels = 0, as_channels = 0;
	ulong gp_waiting = 0, as_waiting = 0;
	ulong gp_delayed = 0, as_delayed = 0;
	ulong blocked_users = 0; // AS events can't block as queue is infinite

	ulong gp_delayed_time = 0;
	ulong as_delayed_time = 0;

	events = list_add(events, 0.0L, EVENT_START);
	++total_events;

	double aux_time;
	int aux_type;

	for (double sim_time; total_events < max_events; total_events++) {
		sim_time = events->time;

		// saving event values and remove that event after
		aux_time = events->time;
		// arrival_time = events->_arrival_time;
		aux_type = events->type;
		next_event(events);


		switch (aux_type /*& EVENT_END*/) {
			case EVENT_START:
				printf("EVENT GP START: %.2lf\n", sim_time);
				++gp_events;

				// If Channels arent full
				if (gp_channels < GP_MAX_CHANNELS) {
					++gp_channels; //A channel servers a call

					//Calcula partida do evento
					if (is_as()){
						double dur = poisson_random(AS_AVG_TIME, AS_STDDEV);
						dur = dur < AS_MIN_TIME? AS_MIN_TIME: dur;
						dur = dur > AS_MAX_TIME? AS_MAX_TIME: dur;
						events = list_add(events, sim_time + dur, EVENT_AS | EVENT_START);
					}else{
						double dur = GP_MIN_TIME + erlang_random(1, GP_AVG_TIME);
						events = list_add(events, sim_time + (dur > GP_MAX_TIME? GP_MAX_TIME: dur), EVENT_END);
					}
				} else if (gp_waiting < BUFFER_SIZE && BUFFER_SIZE > 0){ //If SPACE in BUFFER
					//printf("all channels are busy.\n\t\t");
					gp_buffer = list_add(gp_buffer, sim_time, EVENT_START);
					++gp_waiting;
					++gp_delayed;
				} else {
					//printf("Queue is full. User dropped (%lu)\n", blocked_users);
					++blocked_users;
				}
				// Calcula chegada de proximo evento
				;double dur = erlang_random(1, LAMBDA);
				events = list_add(events, sim_time + dur, EVENT_START);
				break;

			case EVENT_END:
				printf("EVENT GP END: %.2lf\n", sim_time);
				--gp_channels; //Releases 1 channel
				if (gp_waiting > 0 /*&& gp_buffer != NULL*/) {
					--gp_waiting;
					++gp_channels; //A channel servers a call

					double waited = sim_time - gp_buffer->time;
					gp_delayed_time += waited;
					next_event(gp_buffer);

					if (is_as()){
						double dur = poisson_random(AS_AVG_TIME, AS_STDDEV);
						dur = dur < AS_MIN_TIME? AS_MIN_TIME: dur;
						dur = dur > AS_MAX_TIME? AS_MAX_TIME: dur;
						events = list_add(events, sim_time + dur, EVENT_AS | EVENT_START);
					} else {
						double dur = GP_MIN_TIME + erlang_random(1, GP_AVG_TIME);
						events = list_add(events, sim_time + (dur > GP_MAX_TIME? GP_MAX_TIME: dur), EVENT_END);
					}
				}
				break;


			case EVENT_AS | EVENT_START: //(10) = 2
				printf("EVENT AS START: %.2lf\n", sim_time);
				as_events++;

				// If Free Channels
				if (as_channels < AS_MAX_CHANNELS) {


					as_channels++; //A channel servers a call
					gp_channels--; // One channel from GP becomes free, because passes the call to AS

					double as_dur = AS_SS_MIN + erlang_random(1, AS_SS_AVG);
					events = list_add(events, sim_time + as_dur, EVENT_AS | EVENT_END);

					/*if (is_as()){	// // os únicos eventos que geram novas chegadas são os de GP, do tipo START
						double dur = AS_MIN_TIME + erlang_random(1, AS_AVG_TIME);
						events = list_add(events, sim_time + (dur > AS_MAX_TIME? AS_MAX_TIME: dur), EVENT_AS | EVENT_START);
					} else {
						double dur = GP_MIN_TIME + erlang_random(1, GP_AVG_TIME);
						events = list_add(events, sim_time + (dur > GP_MAX_TIME? GP_MAX_TIME: dur), EVENT_END);
					}*/

				} else {
					// ADD TO BUFFER
					as_buffer = list_add(as_buffer, sim_time, EVENT_AS | EVENT_START);
					++as_waiting;
					++as_delayed;
				}
				break;

			case EVENT_AS | EVENT_END:  //(11) = 3
				printf("EVENT AS END: %.2lf\n", sim_time);
				as_channels--;

				if (as_waiting > 0 /*&& as_buffer != NULL*/) {
					--as_waiting;
					as_channels++;
					gp_channels--; // One channel from GP becomes free, because passes the call to AS

					double dur = AS_SS_MIN + erlang_random(1, AS_SS_AVG);
					events = list_add(events, sim_time + dur, EVENT_AS | EVENT_END);

					// double waited = sim_time - as_buffer->time;
					// as_delayed_time += waited;
					next_event(as_buffer);

					/*if (is_as()){	// os únicos eventos que geram novas chegadas são os de GP, do tipo START
						double dur = AS_MIN_TIME + erlang_random(1, AS_AVG_TIME);
						events = list_add(events, sim_time + (dur > AS_MAX_TIME? AS_MAX_TIME: dur), EVENT_AS | EVENT_START);
					} else {
						double dur = GP_MIN_TIME + erlang_random(1, GP_AVG_TIME);
						events = list_add(events, sim_time + (dur > GP_MAX_TIME? GP_MAX_TIME: dur), EVENT_END);
					}*/
				}
				break;
			default:
				printf("Unkown event.\n Exiting...\n");
				exit(EXIT_FAILURE);
		}
	}

	printf("total users: %lu (%.2lf%% as events)\n", total_events, (double)as_events*100/total_events);
	printf("gp delayed users: %lu (%.2lf%%)\n", gp_delayed, (double)gp_delayed*100/total_events);
	printf("gp blocked users: %lu (%.2lf%%)\n", blocked_users, (double)blocked_users*100/total_events);

	printf("gp avg delay: %.2lf seconds\n", (double)(gp_delayed_time)/gp_delayed);
	printf("as avg delay: %.2lf seconds\n", (double)(as_delayed_time)/as_delayed);

	return 0;
}
