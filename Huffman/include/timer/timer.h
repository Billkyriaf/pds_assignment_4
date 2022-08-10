#ifndef HUFFMAN_TIMER_H
#define HUFFMAN_TIMER_H

#include <sys/time.h>

typedef struct timer{
    struct timespec start;
    struct timespec stop;
    long elapsed_sec;
    long elapsed_ms;
    long elapsed_us;
    long elapsed_ns;

}Timer;


/**
 * Store the starting moment in the start struct
 * @param timer  The timer struct
 */
void startTimer(Timer *timer);


/**
 * Store the stopping moment in the stop struct
 * @param timer  The timer struct
 */
void stopTimer(Timer *timer);


/**
 * Print the elapsed time
 * @param timer  The timer struct
 */
void displayElapsed(Timer *timer);

#endif
