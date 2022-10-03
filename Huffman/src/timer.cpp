#include <iostream>
#include "timer.h"


/**
 * Calculates the elapsed time using the start and stop timespec struct of the timer
 *
 * @param timer  The timer struct
 */
void elapsedTime(Timer *timer) {
    timer->elapsed_sec = timer->stop.tv_sec - timer->start.tv_sec;  // Get the hole number of seconds

    long ns = 0;

    if (timer->stop.tv_nsec < timer->start.tv_nsec){  // If a hole second has not passed (eg 5.2s to 6.1s)
        timer->elapsed_sec--; // decrease the seconds
        ns = 1000000000 - timer->start.tv_nsec + timer->stop.tv_nsec;  // and get the nanoseconds passed

    } else if (timer->stop.tv_nsec > timer->start.tv_nsec) { // Else if a hole second has passed (eg 5.2s to 6.3s)
        ns = timer->stop.tv_nsec - timer->start.tv_nsec;  // get the nanoseconds elapsed
    }

    // Extract the micro and milli seconds from the nano seconds

    /*
     * Every 1000ns is 1us
     * Every 1000us is 1ms
     * Every 1000ms is 1s
     */
    if (ns < 1000) {
        timer->elapsed_ms = 0;
        timer->elapsed_us = 0;
        timer->elapsed_ns = ns;

    } else {
        long us = ns / 1000;
        timer->elapsed_ns = ns % 1000;

        if (us < 1000) {
            timer->elapsed_ms = 0;
            timer->elapsed_us = us;

        } else {
            timer->elapsed_ms = us / 1000;
            timer->elapsed_us = us % 1000;
        }
    }
}

/**
 * Store the starting moment in the start struct
 * @param timer  The timer struct
 */
void startTimer(Timer *timer) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &timer->start);
}

/**
 * Store the stopping moment in the stop struct
 * @param timer  The timer struct
 */
void stopTimer(Timer *timer) {
    clock_gettime(CLOCK_MONOTONIC_RAW, &timer->stop);

    elapsedTime(timer);
}

/**
 * Print the elapsed time
 * @param timer  The timer struct
 */
void displayElapsed(Timer *timer) {

    std::cout << timer->elapsed_sec << "s " << timer->elapsed_ms << "ms " << timer->elapsed_us
              << "us " << timer->elapsed_ns << "ns\n" << std::endl;
}