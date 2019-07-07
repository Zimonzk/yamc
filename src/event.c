#include "event.h"

#include "zimonzk/lists.h"
#include <string.h>

arraylist eventindex;

struct event_handler_compound {
	event_handler cb;
	void *userdata;
}

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
		/* probably via queues of function pointers
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

