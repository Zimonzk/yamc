#include "plogger.h"
#include "settings.h"
#include "confconfig.h"
#include "toolbox.h"

#include <stdlib.h>

struct gamesettings gamesettings = {};

static void float_deserializer(char *value, void *data)
{
	float *result = (float*) data;
	*result = atof(value);
	tlog(5, "Deserializing \"%s\" to (float) %f", value, *result);
}

static void int_deserializer(char *value, void *data)
{
	int *result = (int*) data;
	*result = atoi(value);
	tlog(5, "Deserializing \"%s\" to (int) %i", value, *result);
}

void init_settings()
{
	struct confstate cs = {};

	tlog(2, "Initializing settings...");

	conf_register_key(&cs, "FOV", float_deserializer,
			&gamesettings.videosettings.fov);
	conf_register_key(&cs, "height", int_deserializer,
			&gamesettings.videosettings.height);
	conf_register_key(&cs, "width", int_deserializer,
			&gamesettings.videosettings.width);
	conf_register_key(&cs, "fullscreen", int_deserializer,
			&gamesettings.videosettings.fullscreen);
	conf_register_key(&cs, "VSync", int_deserializer,
			&gamesettings.videosettings.vsync);

	conf_register_key(&cs, "master_volume", float_deserializer,
			&gamesettings.audiosettings.master_volume);
	conf_register_key(&cs, "music_volume", float_deserializer,
			&gamesettings.audiosettings.music_volume);
	
	
	if(conf_parse_file(&cs, "config/options.conf") !=
			CONFCONFIG_ERROR_NO_ERROR) {
		yamc_terminate(-123,
				"Could not load game settings"
				"(config/options.conf)");
	}

	conf_destroy_state(&cs);
}
