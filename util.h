#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

void util_fatal(char *msg, char *file, unsigned line);
void util_warn(char *msg, char *file, unsigned line);

void util_restoreTermAtSignal(int sig);
int util_ttyraw(void);

#define FATAL(MSG)      util_fatal((MSG), __FILE__, __LINE__)
#define WARN(MSG)       util_warn((MSG), __FILE__, __LINE__)
#define UNUSED(X)       (void)(X); /*acknowledge that a variable is unused*/

#ifdef __cplusplus
}
#endif

#endif
