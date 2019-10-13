#include "confconfig.h"

#include <stdio.h>
#include <string.h>
#include "plogger.h"

int conf_register_key(struct confstate *cs, char *key, key_callback kcb, void *userdata)
{
	if(cs->keys_inited == 0) {
		arraylist_init(&cs->keys, sizeof(char *), 1);
		cs->keys_inited = 1;
	}
	if(cs->key_callbacks_inited == 0) {
		arraylist_init(&cs->key_callbacks, sizeof(key_callback), 1);
		cs->key_callbacks_inited = 1;
	}
	if(cs->userdatas_inited == 0) {
		arraylist_init(&cs->userdatas, sizeof(void *), 1);
		cs->userdatas_inited = 1;
	}
	arraylist_append(&cs->keys, (void *)&key);
	arraylist_append(&cs->key_callbacks, (void *)&kcb);
	arraylist_append(&cs->userdatas, (void *)&userdata);

	return CONFCONFIG_ERROR_NO_ERROR;
}

int conf_parse_file(struct confstate *cs, char *path)
{
	FILE *conffile;
	char *key, *value;
	int assigned;

	conffile = fopen(path, "r");
	if(conffile == 0) {
		return CONFCONFIG_ERROR_NOFILE;
	}

	while((assigned = fscanf(conffile, " %m[^ \n\t\f\r\v=] = %m[^\n]", &key, &value))
		       	!= EOF) {
		if(assigned == 0) {
			tlog(5, "skipping beause no match");
			continue;
		}
		if(assigned == 1) {
			tlog(5, "skipping because only one match");
			free(key);
			continue;
		}
		/*tlog(5, "read %s, %s", key, value);*/
		for(int i = 0; i < cs->keys.used_units; i++) {
			key_callback kcb = *(key_callback *)arraylist_get(&cs->key_callbacks, i);
			if(strcmp(key, *(char **)arraylist_get(&cs->keys, i)) == 0) {
				kcb(value, *(void **)arraylist_get(&cs->userdatas, i));
				break;
			}
		}
		free(key);
		free(value);
	}
	fclose(conffile);
	return CONFCONFIG_ERROR_NO_ERROR;
}

void conf_destroy_state(struct confstate *cs)
{
	arraylist_delete(&cs->keys);
	arraylist_delete(&cs->key_callbacks);
	arraylist_delete(&cs->userdatas);
}

