/* wrapper to make the logger library threadsafe */
#ifndef __PLOGGER_H_INCLUDED__
#define __PLOGGER_H_INCLUDED__

#define _GNU_SOURCE

#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include "zimonzk/logger.h"

extern pthread_mutex_t logging_mutex;

#define tset_verbosity(verbosity)\
	pthread_mutex_lock(&logging_mutex);\
	set_verbosity(verbosity);\
	pthread_mutex_unlock(&logging_mutex);

#define tlog(verbosity, fmt, ...){\
	pid_t tid = syscall(SYS_gettid);\
	pthread_mutex_lock(&logging_mutex);\
	printf("\x1B[1;35m#%u \x1B[0m", tid);\
	zlog(verbosity, fmt, ##__VA_ARGS__);\
	pthread_mutex_unlock(&logging_mutex);}

#define twarn(fmt, ...){\
	pid_t tid = syscall(SYS_gettid);\
	pthread_mutex_lock(&logging_mutex);\
	printf("\x1B[1;35m#%u \x1B[0m", tid);\
	warn(fmt, ##__VA_ARGS__);\
	pthread_mutex_unlock(&logging_mutex);}

#define terror(fmt, ...){\
	pid_t tid = syscall(SYS_gettid);\
	pthread_mutex_lock(&logging_mutex);\
	fprintf(stderr, "\x1B[1;35m#%u \x1B[0m", tid);\
	error(fmt, ##__VA_ARGS__);\
	pthread_mutex_unlock(&logging_mutex);}


/* fatal doesn't get wrapped, since it ends the program
 * so i don't care about thread safety */

#endif