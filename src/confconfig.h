#ifndef __CONFCONFIG_H_INCLUDED__
#define __CONFCONFIG_H_INCLUDED__

#include "zimonzk/lists.h"

#define CONFCONFIG_ERROR_NO_ERROR 0
#define CONFCONFIG_ERROR_NOFILE -1

struct confstate {
	arraylist keys;
	arraylist key_callbacks;
	arraylist userdatas;
	char keys_inited;
	char key_callbacks_inited;
	char userdatas_inited;
};
/* the string in "value" is not guranteed to be kept in memory
 * after the callback returns. */
typedef void (*key_callback)(char* value, void *userdata);

/* YOU need to keep the string that "key" is reffering to in memory, I don't copy it.
 * 
 * Registering multiple keys which are the same (either in value or in content) will
 * cause only the first callback to be called.
 * Keys may contain any character except whitespaces and '='. 
 * Values however can contain any character except newline */
int conf_register_key(struct confstate *cs, char *key, key_callback kcb, void *userdata);

int conf_parse_file(struct confstate *cs, char *path);

void conf_destroy_state(struct confstate *cs);

#endif
