#include <stdlib.h>
#include <string.h>
#include "_io.h"
#include "numbers.h"
#include "letters.h"
#include "ht1632c.h"
#include "timers.h"
#include "mq.h"

bool last_state_down[3] = {false, false, false};
bool steady_state[3] = {true, true, true};
bool key_repeating[3] = {false, false, false};
uint16_t state_change_clock[3] = {0, 0, 0};
uint16_t repeat_initial_delay[3] = {300, 300, 300};
uint16_t repeat_subsequent_delay[3] = {200, 50, 200};

void init_keys(void)
{
    /* Set high 3 bits of port D as input */
    DDRD &= 0b00011111;

    /* Turn on pull-up resistors on high 3 bits */
    PORTD |= 0b11100000;

    DDRC |= 1 << 5;
}

void handle_keys(void)
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

bool render_number(uint32_t number, byte board[32])
{
    ldiv_t q;
    const character *c;
    uint8_t pos = 0;
    int8_t i;
    uint32_t n = number;

    // calculate position of least significant digit
    do {
        q = ldiv(n, 10);
        n = q.quot;
        c = &numbers[q.rem];
        pos += c->columns + 1;
    } while (n);
    pos--;

    if (pos > 31)
        // overflows board
        return false;

    // render
    do {
        q = ldiv(number, 10);
        number = q.quot;
        c = &numbers[q.rem];
        for (i = c->columns - 1; i >= 0; i--) {
            board[pos--] = pgm_read_byte(&(c->bitmap[i]));
        }
        pos--;
    } while (number);

    return true;
}

bool render_string(const char* string, byte board[32])
{
    uint8_t pos = 1, i;
    const character *c;
    while (*string) {
        if (*string == ' ')
            pos += 3;
        else if (*string < 'A' || *string > 'Z')
            return false;
        else {
            c = &letters[*string - 65];
            for (i = 0; i < c->columns; i++) {
                board[pos++] = pgm_read_byte(&(c->bitmap[i]));
                if (pos > 31)
                    return false;
            }
            pos++;
        }
        if (pos > 31)
            return false;
        string++;
    }

    return true;
}

void read_string(char* str, uint8_t length, uint8_t timer)
{
    message_t message;
    char* cursor = str;
    char current_char = 'A';
    bool redraw = true;

    memset(str, 0, length + 1);
    *cursor = 'A';
    set_timer(250, timer, true);

    while (length) {
        if (mq_get(&message)) {
            if (msg_get_event(message) == M_TIMER && msg_get_param(message) == 0) {
                if (*cursor)
                    *cursor = 0;
                else
                    *cursor = current_char;

                redraw = true;
            }
            else if (msg_get_event(message) == M_KEY_DOWN || msg_get_event(message) == M_KEY_REPEAT) {
                switch (msg_get_param(message)) {
                case KEY_LEFT:
                    if (current_char > 'A')
                        current_char--;
                    break;
                case KEY_RIGHT:
                    if (current_char < 'Z')
                        current_char++;
                    break;
                }
                *cursor = current_char;
                set_timer(250, timer, true);
                redraw = true;
            }
            if (msg_get_event(message) == M_KEY_DOWN && msg_get_param(message) == KEY_MIDDLE) {
                *cursor = current_char;
                length--;
                if (length) {
                    cursor++;
                    current_char = 'A';
                    *cursor = current_char;
                    redraw = true;
                }
            }

            if (redraw) {
                memset(leds, 0, 32);
                render_string(str, leds);
                HTsendscreen();
                redraw = false;
            }
        }
    }
    stop_timer(timer);
    memset(leds, 0, 32);
    HTsendscreen();
}
