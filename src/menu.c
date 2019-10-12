#include "plogger.h"
#include "menu.h"
#include "gui.h"
#include "event.h"

extern struct event_index_card *ic_reg;

void ontestbutton(void* userdata)
{
	struct event_index_card *ic = userdata;
	tlog(5, "ACTION");
	trigger_event(ic, "came from testbutton! xD");
	//SDL_Quit();
	//exit(0);
}


void build_esc_menu()
{
	/*test button*/
	gui_add_button(-0.3f, 0.55f, 0.5f, 96.0f/480.0f, "TEST",
		       ontestbutton, (void *)ic_reg);


}
