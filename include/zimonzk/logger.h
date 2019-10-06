/** @file 
 *  \brief interface to the logging utilities 
 *
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern int global_verbosity;

/** \brief Sets the logging verbosity forthe programm
 *
 * @param v [in] The verbosity level, higher values mean mor verbose reporting.
 *               Set to -1 to enable all messages.
 */
void set_verbosity(int v);

/** \brief Logs a message
 *
 *  @param verbosity [in] The minimum verbosity required to show this message.
 *  @param fmt [in] The message format (as in printf, ...).
 *  @param ... [in] parameters (as in printf, ...)
 */
void zlog(int verbosity, char* fmt, ...);

/** \brief Prints a warning.
 *
 *  @param fmt [in]  The message format (as in prntf, ...).
 *  @param ... [in] parameters (as in printf, ...)
 */
void warn(char* fmt, ...);

/** \brief Logs an error to stderr.
 *
 *  @param fmt [in]  The message format (as in prntf, ...).
 *  @param ... [in] parameters (as in printf, ...)
 */
void error(char* fmt, ...);

/** \brief Shuts the program down with a niche message.
 *
 *  @param fmt [in]  The message format (as in prntf, ...).
 *  @param ... [in] parameters (as in printf, ...)
 */
#define fatal(fmt, ...) f_fatal(__LINE__, __FILE__, fmt, ##__VA_ARGS__);
void f_fatal(unsigned long line, char* file, char* fmt, ...);
