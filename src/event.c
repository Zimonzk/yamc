#include "event.h"

#include "zimonzk/lists.h"
#include <string.h>
#include <pthread.h>

arraylist eventindex;

static arraylist jobs;
static char jobs_inited = 0;
static pthread_mutex_t jobs_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t jobs_cond = PTHREAD_COND_INITIALIZER;

struct event_handler_compound {
	event_handler cb;
	void *userdata;
};

struct job_compound {
	event_handler cb;
	const struct evet_index_card *ic;
	void *userdata;
	void *eventdata;
};

struct event_index_card *register_event(struct event_index_card *ic)
{
	static char eventindex_inited = 0;

	if(!eventindex_inited) {
		arraylist_init(&eventindex, sizeof(event_index_card), 1);
	}

	arraylist_append(eventindex, ic);

	ic = arraylist_get(&eventindex, eventindex.used_units - 1);

	arraylist_init(&ic->handlers, sizeof(struct event_handler_compound), 1);

	return ic;
}

int register_event_handler(const char *eventname, event_handler cb,
				void *userdata)
{
	struct event_handler_compound ehc = {.cb = cb, .userdata = userdata};
	for(int i = 0; i < eventindex.used_units; i++) {
		struct event_index_card *ic = arraylist_get(&eventindex, i);
		const char *checkname = ic->name;
		if(strcmp(eventname, d *register_event(struct event_index_card *ic)
{
	static char eventindex_inited = 0;

	if(!eventindex_inited) {
		arraylist_init(&eventindex, sizeof(event_index_card), 1);
	}

	arraylist_append(eventindex, ic);

	ic = arraylist_get(&eventindex, eventindex.used_units - 1);

	arraylist_init(&ic->handlers, sizeof(struct event_handler_compound), 1);

	return ic;
}

int register_event_handler(const char *eventname, event_handler cb,
				void *userdata)
{
	struct event_handler_compound ehc = {.cb = cb, .userdata = userdata};
	for(int i = 0; i < eventindex.used_units; i++) {
		struct event_index_card *ic = arraylist_get(&eventindex, i);
		const char *checkname = ic->name;
		if(strcmp(eventname, checkname) == 0) {
			arraylist_append(&ic->handlers, &ehc);
			return 1; /* means success */
		}
	}
	return 0; /* means failure */
}

int trigger_event(struct event_index_card *ic, void *eventdata) {
	for(int i = 0; i < ic->handlers.used_units; i++) {
		/* spread onto threads */
		/* via a queue of function pointers
		 * and a way to wake up a thread
		 * that has been waiting for work */
	}
	return 1; /* means success */
}

void main_loop_sync_slot(void)
{
	/* free a mutex that is being waited upon by all threads that
	 * called frame_sync_begin. */

	/* then wait for them to release it via frame sync end */
}

void frame_sync_begin(void)
{
	/* wait for the mutex that is released by main_loop_sync_slot */
}

void frame_sync_end(void)
{
	/* release the mutex */
}

static *void worker_func(void *data)
{
	struct job_compound jc;
	while(1) {
		jc = deq_job();
		jc.cb(jc.ic, jc.eventdata, jc.userdata);
	}
	return NULL;
}

enq_job(struct job_compound)
{
	pthread_mutex_lock(&jobs_mutex);
	if(!jobs_inited) {
		arraylist_init(&jobs, sizeof(struct job_compound), 16);
	}
	arraylist_append(&jobs, &job_compound);
	/*TODO signal potential waiting threads to continue */
	/*only if some are waiting?*/
	pthread_mutex_unlock(&jobs_mutex);
}
struct job_compound deq_job(void)
{
	struct job_compound jc;
	pthread_mutex_lock(&jobs_mutex);

	while(!jobs_inited || jobs.used_units == 0) {
		pthread_mutex_unlock(&jobs_mutex);
		/*TODO wait for job */
		pthread_cond_wait(&jobs_cond, &jobs_mutex);
		/*waiting workers counter?*/
	}
	memcpy(&jc, arraylist_get(&jobs, jobs.used_units - 1),
		sizeof(struct job_compound));
	arraylist_del_element(&jobs, jobs.used_units - 1);
	
	pthread_mutex_unlock(&jobs_mutex);
	return jc;
}
