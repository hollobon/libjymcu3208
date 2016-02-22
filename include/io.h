#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include "timers.h"
#include "mq.h"
#include "_io.h"

extern bool last_state_down[3];
extern bool steady_state[3];
extern bool key_repeating[3];
extern uint16_t state_change_clock[3];
extern uint16_t repeat_initial_delay[3];
extern uint16_t repeat_subsequent_delay[3];

inline void handle_keys(void) __attribute__((always_inline));

inline void handle_keys(void)
{
    for (uint8_t key = 0; key < 3; key++) {
        bool down = key_down(key);

        if (down != last_state_down[key]) {
            // State change
            last_state_down[key] = down;
            steady_state[key] = false;
            state_change_clock[key] = clock_count;
        }
        else if (down && (clock_count - state_change_clock[key] > (key_repeating[key] ? repeat_subsequent_delay[key] : repeat_initial_delay[key]))) {
            // Autorepeat
            state_change_clock[key] = clock_count;
            mq_put(msg_create(M_KEY_REPEAT, key));
            key_repeating[key] = true;
        }
        else if (!steady_state[key] && clock_count - state_change_clock[key] > DEBOUNCE_TIME) {
            // Debounce period over and key in same state
            mq_put(msg_create(down ? M_KEY_DOWN : M_KEY_UP, key));
            steady_state[key] = true;
            if (!down)
                key_repeating[key] = false;
        }
    }
}

#endif /* IO_H */
