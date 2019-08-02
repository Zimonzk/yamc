#include "event.h"

#include "zimonzk/lists.h"
#include <string.h>
#include <pthread.h>

arraylist eventindex;

static arraylist jobs;
static char jobs_inited = 0;
static unsigned int waiting_workers = 0;
static pthread_mutex_t jobs_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t jobs_cond = PTHREAD_COND_INITIALIZER;


static unsigned int syncwaiters = 0; /* the amount of threads waiting for a sync slot */
static unsigned int pre_syncwaiters = 0; /* the amount of threads wanting to wait
					  * fot a sync slot but
					  * the synced threads are not all done yet. */
static unsigned char syncers_executing = 0; /* wether the synced thready are executing
					     * atm */
static pthread_mutex_t sync_meta_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sync_meta_cond = PTHREAD_COND_INITIALIZER;


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

struct event_index_card *register_event(struct event_index_card *ic) {
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
	/* broadcast a condition variable and then wait
	 * until all synchronizations have been done */
	/* the check for all synchronistions is via an integer, which holds
	 * the cont of threads that still wait for their sync slot. */
	pthread_mutex_lock(&sync_meta_mutex);
	pthread_cond_broadcast(&sync_meta_cond);
	syncers_executing = 1;

	pthread_cond_wait(&sync_meta_cond, &sync_meta_mutex);
	pthread_mutex_unlock(&sync_meta_mutex);
}

void frame_sync_begin(void)
{
	pthread_mutex_lock(&sync_meta_mutex);
	if(syncers_executing) { /* this entire construct is supposed to prevent
				 * threads that aquired the mutex while others
				 * are still competing for it in order to complete
				 * the sync slot from increasing the waiters while
				 * it would be impossible for them to decrease the
				 * waiters count because the cond_wait will only
				 * trigger in the next slot. */
		pre_syncwaiters++;
	} else {
		syncwaiters++;
	}
	pthread_cond_wait(&sync_meta_cond, &sync_meta_mutex);
}

void frame_sync_end(void)
{
	if(--syncwaiters == 0) {
		syncers_executing = 0;
		syncwaters = pre_syncwaiters;
		pre_syncwaiters = 0;

		pthread_cond_broadcast(&sync_meta_cond);
	}
	pthread_mutex_unlock(&sync_meta_mutex);
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

void enq_job(struct job_compound)
{
	pthread_mutex_lock(&jobs_mutex);
	if(!jobs_inited) {
		arraylist_init(&jobs, sizeof(struct job_compound), 16);
	}
	arraylist_append(&jobs, &job_compound);
	/* signal potential waiting threads to continue */
	/* only if some are waiting */
	if(waiting_workers > 0) {
		pthread_cond_broadcast(&jobs_cond);
		waiting_workers == 0;
	}
	pthread_mutex_unlock(&jobs_mutex);
}

struct job_compound deq_job(void)
{
	struct job_compound jc;
	pthread_mutex_lock(&jobs_mutex);

	while(!jobs_inited || jobs.used_units == 0) {
		/* waiting workers counter */
		waiting_workers++;
		/* wait for job */
		pthread_cond_wait(&jobs_cond, &jobs_mutex);
	}
	memcpy(&jc, arraylist_get(&jobs, jobs.used_units - 1),
		sizeof(struct job_compound));
	arraylist_del_element(&jobs, jobs.used_units - 1);
	
	pthread_mutex_unlock(&jobs_mutex);
	return jc;
}
