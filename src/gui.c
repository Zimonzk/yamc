#include "gui.h"
#include "zimonzk/lists.h"
#include "texture.h"
#include "shader.h"
#include "fonter.h"

#include <string.h>
#include <SDL2/SDL.h>

static float button_start_vps[] = {
	0.0f, 0.0f,
	0.5f, 0.0f,
	0.5f, 1.0f,

	0.0f, 0.0f,
	0.5f, 1.0f,
	0.0f, 1.0f
};
static float button_start_uvs[] = {
	0.0f, 1.0f - 12.0f/64.0f,
	6.0f/64.0f, 1.0f - 12.0f/64.0f,
	6.0f/64.0f, 1.0f,

	0.0f, 1.0 - 12.0f/64.0f,
	6.0f/64.0f, 1.0f,
	0.0f, 1.0f
};
static float button_middle_vps[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,

	0.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f
};
static float button_middle_uvs[] = {
	6.0f/64.0f, 1.0f - 12.0f/64.0f,
	18.0f/64.0f, 1.0f - 12.0f/64.0f,
	18.0f/64.0f, 1.0f,

	6.0f/64.0f, 1.0f - 12.0f/64.0f,
	18.0f/64.0f, 1.0f,
	6.0f/64.0f, 1.0f
};
static float button_end_vps[] = {
	0.0f, 0.0f,
	0.5f, 0.0f,
	0.5f, 1.0f,

	0.0f, 0.0f,
	0.5f, 1.0f,
	0.0f, 1.0f

};
static float button_end_uvs[] = {
	18.0f/64.0f, 1.0f - 12.0f/64.0f,
	24.0f/64.0f, 1.0f - 12.0f/64.0f,
	24.0f/64.0f, 1.0f,

	18.0f/64.0f, 1.0f - 12.0f/64.0f,
	24.0f/64.0f, 1.0f,
	18.0f/64.0f, 1.0f
};

struct button {
	float pos[2];
	float size[2];
	const char *label;
	buttoncb cb;
	void *userdata;
	char state;
};

static arraylist buttons;
static GLuint textureID_atlas;
static GLuint programID_gui;
static GLuint uniformID_scale;
static GLuint uniformID_start;
static GLuint uniformID_sampler;
static GLuint bufferID_vertex;
static GLuint bufferID_uv;

void init_gui(void)
{
	arraylist_init(&buttons, sizeof(struct button), 1);
	textureID_atlas = texture_from_png("textures/ui/menu.png");
	if(textureID_atlas == 0) {
		SDL_Log("Could not load menu atlas.");
		exit(1);
	}
	glBindTexture(GL_TEXTURE_2D, textureID_atlas);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	programID_gui = LoadShaders("shaders/GUI.vertexshader", "shaders/GUI.fragmentshader");
	uniformID_scale = glGetUniformLocation(programID_gui, "scale");
	uniformID_start = glGetUniformLocation(programID_gui, "start");
	uniformID_sampler = glGetUniformLocation(uniformID_sampler, "my_sampler");

	glGenBuffers(1, &bufferID_vertex);
	glGenBuffers(1, &bufferID_uv);
}

void gui_render(void)
{
	glUseProgram(programID_gui);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID_atlas);
	glUniform1i(uniformID_sampler, 0);
	/* for all buttons, divide into sections that can use a continuous area on the atlas,
	 * write their vertices into a vertexbuffer and their uvs into a uvbuffer */
	for(int i = 0; i < buttons.used_units; i++) {
		float cursor = 0.0f;
		float vps[12], uvs[12];
		struct button *button = arraylist_get(&buttons, i);
		arraylist vpb, uvb;
		arraylist_init(&vpb, 12 * sizeof(float), 8);
		arraylist_init(&uvb, 12 * sizeof(float), 8);

		glUniform2f(uniformID_start, button->pos[0], button->pos[1]);
		/*scale is determined by the size_y, size_x will be filled by repeating the middle section. */
		glUniform2f(uniformID_scale, button->size[1]*480.0f/640.0f, button->size[1]);

		
		/* generate start piece */
		arraylist_append(&vpb, button_start_vps);

		memcpy(uvs, button_start_uvs, 12 * sizeof(float));
		if(button->state == 1) {
			for(int i = 0; i < 6; i++) {
				uvs[2 * i] += 24.0f/64.0f;
			}
		}
		arraylist_append(&uvb, uvs);

		cursor = 0.5f;

		while((cursor + 0.5f) * button->size[1] * 480.0f/640.0f < button->size[0]) {
			/* generate one piece of the niddle */
			memcpy(vps, button_middle_vps, 12 * sizeof(float));

			for(int i = 0; i < 6; i++) {
				vps[2 * i] += cursor;
			}

			arraylist_append(&vpb, vps);

			memcpy(uvs, button_middle_uvs, 12 * sizeof(float));
			if(button->state == 1) {
				for(int i = 0; i < 6; i++) {
					uvs[2 * i] += 24.0f/64.0f;
				}
			}
			arraylist_append(&uvb, uvs);

			cursor += 1.0f;
		}
		/* generate end piece */
		memcpy(vps, button_end_vps, 12 * sizeof(float));

		for(int i = 0; i < 6; i++) {
			vps[2 * i] += cursor;
		}
		arraylist_append(&vpb, vps);


		memcpy(uvs, button_end_uvs, 12 * sizeof(float));
		if(button->state == 1) {
			for(int i = 0; i < 6; i++) {
				uvs[2 * i] += 24.0f/64.0f;
			}
		}
		arraylist_append(&uvb, uvs);

		/* update button size to actual size */
		button->size[0] = (cursor + 0.5f) * button->size[1] * 480.0f/640.0f;



		glBindBuffer(GL_ARRAY_BUFFER, bufferID_vertex);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 * vpb.used_units, vpb.data, GL_STREAM_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindBuffer(GL_ARRAY_BUFFER, bufferID_uv);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 * uvb.used_units, uvb.data, GL_STREAM_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glDisable(GL_DEPTH_TEST);

		glDrawArrays(GL_TRIANGLES, 0, vpb.used_units * 6);

		arraylist_delete(&vpb);
		arraylist_delete(&uvb);

		/* draw the label centered */
		render_text(button->label, button->pos[0] + (cursor + 0.5f) * button->size[1] * 0.5f * 480.0f/640.0f - 32.0f/640.0f * 0.5f * strlen(button->label),
				button->pos[1] + button->size[1] * 0.5f - 0.5f * 32.0f/480.0f);

	}
}

void gui_input(int x, int y, char down)
{
	/* Down means button is pressed.
	 * When it is 1 it should change the buttons appereance.
	 * When it is 0 it changes the appereance back and calls the callback. */
	/* gl-ify x and y */
	float fx = x * 2/640.0f - 1.0f;
	float fy = (480 - y) * 2/480.0f - 1.0f;
	/*TODO check for a button which is at the point of click,
	 * do button interaction if one is found */
	for(int i = 0; i < buttons.used_units; i++) {
		struct button *button = arraylist_get(&buttons, i);

		SDL_Log("xypppp: %f %f %f %f %f %f", fx, fy, button->pos[0], button->pos[0] + button->size[1], button->pos[1] + button->size[1]);

		if(fx >= button->pos[0] && fx <= button->pos[0] + button->size[0] && fy >= button->pos[1] && fy <= button->pos[1] + button->size[1]) {
			SDL_Log("CLACK");
			if(down) {
				button->state = 1;
			} else {
				if(button->state == 1) {
					button->state = 0;
					button->cb(button->userdata);
				}
			}
			break;
		} else {
			SDL_Log("NOPE");
		}
	}
}

int gui_add_button(float x, float y, float size_x, float size_y, const char *label, buttoncb bcb, void *userdata)
{
	struct button button;
	button.pos[0] = x;
	button.pos[1] = y;
	button.size[0] = size_x;
	button.size[1] = size_y;
	button.label = label;
	button.cb = bcb;
	button.userdata = userdata;

	arraylist_append(&buttons, (void *)&button);

	return 0;
}

void gui_clear(void)
{
	arraylist_delete(&buttons);
	arraylist_init(&buttons, sizeof(struct button), 1);
}

