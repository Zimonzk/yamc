#ifndef __EVENT_H_INCLUDED__
#define __EVENT_H_INCLUDED__

#include "zimonzk/lists.h"

#define NUM_WORKERS 64

struct event_index_card {
	const char *name; /* unique name use dots for grouping
			   * eg. sys.player.move */
	arraylist handlers; /* will be filled by register_event_handler */
};

typedef void (*event_handler)(const struct event_index_card * ic,
				void* eventdata, void * userdata);

struct job_compound { /* internal struct */
	event_handler cb;
	const struct event_index_card *ic;
	void *userdata;
	void *eventdata;
};


struct event_index_card *register_event(struct event_index_card *ic);

/* the handlers will be executed in parallel. do not male assumptions about
 * sceduling behavior pls. when needed you need to synchronize with other
 * events on your own through eventdata or userdata */
int register_event_handler(const char *eventname, event_handler cb, void *userdata);

int trigger_event(struct event_index_card *event, void *eventdata);

/* this whole thing is probably in need of performance optimization.
 * for examle preventing all worker threads from being locked waiting for the
 * frame to end. */
void main_loop_sync_slot(void); /* internal function to allow syncronisation 
				     with the main loop on demand */
void frame_sync_begin(void); /* call this to block until the frame ends */
void frame_sync_end(void); /* call this to allow the next thread waiting for a
			      slot on the main loop to execute. (or let the
			      main loop itself continue if there are no more
			      waiting threads */
static void *worker_func(void *data); /* internal function */
static void enq_job(struct job_compound); /* internal function */
static struct job_compound deq_job(void); /* internal function */

#endif
