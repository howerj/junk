/**@author Richard James Howe
 *
 * Timer test program, for real world use set the typedefs to use a larger
 * value, for testing a 16-bit value works best as wrap around can be checked
 * for
 *
 * Testing:
 *
 *    make timer
 *    while true; do time ./timer; done
 *
 * 'time' should always report 'timer' being run for 1 second, even when the
 * counter wraps around 
 *
 * NB. If we used an internal fixed point representation for time we could
 * do lossless conversion between seconds and milliseconds, which is always
 * useful.
 *
 * @todo More testing, missed event detection, better API
 */
#include "system.h"
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

typedef uint16_t time_ms_t; /**< time type in milliseconds */

typedef struct {
	time_ms_t period;     /**< period of time until timer expires */
	time_ms_t start;      /**< when the timer was started */
	time_ms_t expired_on; /**< when the timer expired */
	bool expired;         /**< has the timer expired? Latches up to true and stays true until reset */
	bool fired_once;      /**< has the timer expired at least once? */
	bool initialized;     /**< has the timer been initialized? */
} util_timer_t;

/**Get the current time in milliseconds
 * @return current time in milliseconds */
time_ms_t util_time_ms(void);

util_timer_t util_timer_start_ms(time_ms_t period);
bool util_timer_expired(util_timer_t * t);

time_ms_t util_time_ms(void)
{
/**@bug This really should be a monotonic clock source, being lazy here*/
#if 0
#include <sys/time.h>
	struct timeval tp;
	gettimeofday(&tp, NULL);
	time_ms_t ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	return ms;
#else
#include <time.h>
	const clock_t c = clock();
	const double s = ((double)c) / CLOCKS_PER_SEC;
	const double ms = s * 1000.0;
	return ms;
#endif
}

typedef int16_t signed_time_ms_t;
#define TIMER_MAX  (UINT16_MAX)
#define TIMER_HALF ((TIMER_MAX/2)+1)

util_timer_t util_timer_start_ms(time_ms_t period)
{
	assert(period < TIMER_HALF);
	util_timer_t r = {
		.expired = false,
		.period = period,
		.start = util_time_ms(),
		.fired_once = false,
		.expired_on = 0,
		.initialized = true,
	};
	return r;
}

void util_timer_reset(util_timer_t * t)
{
	assert(t && t->initialized);
	t->expired = false;
	/* 't->period' and 't->initialized' stays the same */
	t->start = util_time_ms();
	t->fired_once = false;
	t->expired_on = 0;
}

/* NB. Assumes a sensible calling rate, less than ((maximum value of time_ms_t)/2) milliseconds */
bool util_timer_expired(util_timer_t * t)
{
	assert(t && t->initialized);
	BUILD_BUG_ON(sizeof(signed_time_ms_t) != sizeof(time_ms_t));
	BUILD_BUG_ON((((time_ms_t) - 1) / 2) + 1 != (TIMER_HALF));

	if (t->expired) {
		t->fired_once = true;
		return true;
	}

	const time_ms_t expires = t->start + t->period;
	const time_ms_t now = util_time_ms();

	if ((signed_time_ms_t) (expires - now) <= 0) {
		t->expired_on = now;
		t->expired = true;
	}

	return t->expired;
}

bool util_timer_expired_rising_edge(util_timer_t * t)
{
	assert(t && t->initialized);
	return util_timer_expired(t) && !(t->fired_once);
						    /**@warning The order of execution of this statement matters*/
}

#define MIN(X, Y) ((X) > (Y) ? (Y) : (X))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

/**@todo timer that refires every N cycles taking into account clock jitter to
 * avoid clock drifting */
bool util_timer_expired_cyclic(util_timer_t * t)
{
	assert(t && t->initialized);
	bool r = util_timer_expired_rising_edge(t);
	if (r) {
		const time_ms_t expires = (t->start + t->period);
		time_ms_t jitter =
		    MAX(t->expired_on, expires) - MIN(t->expired_on, expires);
		t->expired = false;
		t->expired_on = 0;
		t->fired_once = false;
		jitter %= t->period;
		t->start = util_time_ms() - jitter;
		//fprintf(stdout, "jit = %lu", (unsigned long)jitter);
	}
	return r;
}

int main(void)
{
	time_ms_t wait = 1000;
	util_timer_t ttt = util_timer_start_ms(wait);
#if 0
	const time_ms_t start = util_time_ms();
	fprintf(stdout, "timer start (ms = %lu)\n", (unsigned long)wait);
	while (!util_timer_expired_rising_edge(&ttt)) ;
	const time_ms_t end = util_time_ms();
	fprintf(stdout, "timer fired (%lu %lu %s)\n", (unsigned long)start,
		(unsigned long)end, end - start > start ? "true" : "false");
#endif
	for (unsigned i = 0;i < 5;) {
		if (util_timer_expired_cyclic(&ttt)) {
			fprintf(stdout, ".\n");
			i++;
		}
	}

	return 0;
}
