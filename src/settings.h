#ifndef __SETTINGS_H_INCLUDED__
#define __SETTINGS_H_INCLUDED__

struct videosettings {
	float fov;
	int height;
	int width;
	int fullscreen;
	int vsync;
};

struct audiosettings {
	float master_volume;
	float music_volume;
};

struct gamesettings {
	struct videosettings videosettings;
	struct audiosettings audiosettings;
};

extern struct gamesettings gamesettings;

void init_settings();

#endif
