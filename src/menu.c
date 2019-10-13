#include "plogger.h"
#include "menu.h"
#include "gui.h"
#include "event.h"
#include "input.h"
#include "settings.h"
#include "toolbox.h"

extern struct event_index_card *ic_reg;

#define MENU_BUTTONS_WIDTH (2.0f * 240.0f / gamesettings.videosettings.width)
#define MENU_BUTTONS_HEIGHT (2.0f * 240.0f / (4.0f * gamesettings.videosettings.height))

/*void ontestbutton(void* userdata)
{
	struct event_index_card *ic = userdata;
	tlog(5, "ACTION");
	trigger_event(ic, "came from testbutton! xD");
	//SDL_Quit();
	//exit(0);
}*/

static void on_continue(void *userdata)
{
	unpause_game();	
}

static void on_options()
{
	/*TODO add optins menu*/
}

static void on_quit()
{
	yamc_terminate(0, "User pressed the \"Quit\" button."
			" TODO: proper quitting, not terminating.");
}

void build_esc_menu()
{
	/*test button*/
	/*gui_add_button(-0.3f, 0.55f, 0.5f, 96.0f/480.0f, "TEST",
		       ontestbutton, (void *)ic_reg);*/

	gui_add_button(-MENU_BUTTONS_WIDTH/2, 0.55f, MENU_BUTTONS_WIDTH,
			MENU_BUTTONS_HEIGHT, "Back to game", on_continue, NULL);
	
	gui_add_button(-MENU_BUTTONS_WIDTH/2,
			0.55f - MENU_BUTTONS_HEIGHT * 1.5f, MENU_BUTTONS_WIDTH,
			MENU_BUTTONS_HEIGHT, "Options", on_options, NULL);

	gui_add_button(-MENU_BUTTONS_WIDTH/2,
			0.55f - MENU_BUTTONS_HEIGHT * 3.0f, MENU_BUTTONS_WIDTH,
			MENU_BUTTONS_HEIGHT, "Quit", on_quit, NULL);



}
