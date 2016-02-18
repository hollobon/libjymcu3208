#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include "timers.h"
#include "mq.h"
#include "_io.h"

extern key_state last_state[3];
extern bool steady_state[3];
extern bool key_repeating[3];
extern uint16_t state_change_clock[3];
extern uint16_t repeat_initial_delay[3];
extern uint16_t repeat_subsequent_delay[3];

inline void handle_keys(void) __attribute__((always_inline));

inline void handle_keys(void)
{
    for (int key = 0; key < 3; key++) {
        if (key_down(key)) {
            if (last_state[key] == up) {
                last_state[key] = down;
                steady_state[key] = false;
                state_change_clock[key] = clock_count;
            }
            else if (clock_count - state_change_clock[key] > (key_repeating[key] ? repeat_subsequent_delay[key] : repeat_initial_delay[key])) {
                state_change_clock[key] = clock_count;
                mq_put(msg_create(M_KEY_REPEAT, key));
                key_repeating[key] = true;
            }
            else if (!steady_state[key] && clock_count - state_change_clock[key] > DEBOUNCE_TIME) {
                mq_put(msg_create(M_KEY_DOWN, key));
                steady_state[key] = true;
            }
        }
        else {
            if (last_state[key] == down) {
                last_state[key] = up;
                steady_state[key] = false;
                state_change_clock[key] = clock_count;
            }
            else if (!steady_state[key] && clock_count - state_change_clock[key] > DEBOUNCE_TIME) {
                mq_put(msg_create(M_KEY_UP, key));
                steady_state[key] = true;
                key_repeating[key] = false;
            }
        }
    }
}

#endif /* IO_H */
