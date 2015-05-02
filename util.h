#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

void util_fatal(char *msg, char *file, unsigned line);
void util_warn(char *msg, char *file, unsigned line);

void util_restoreTermAtSignal(int sig);
int util_ttyraw(void);
int util_getchraw(void);

typedef void* mutex_type;
typedef void* thread_type;

#define FATAL(MSG)      util_fatal((MSG), __FILE__, __LINE__)
#define WARN(MSG)       util_warn((MSG), __FILE__, __LINE__)
#define UNUSED(X)       (void)(X); /*acknowledge that a variable is unused*/

mutex_type util_mutex_create(void);
int util_mutex_lock(mutex_type m);
int util_mutex_unlock(mutex_type m); 
thread_type util_thread_alloc(void);
int util_thread_new(thread_type thread, void *(*routine) (void*), void *args);

#ifdef __cplusplus
}
#endif

#endif
