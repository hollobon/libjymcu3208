#ifndef _IO_H
#define _IO_H

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include "timers.h"
#include "mq.h"

#define key_down(n) ((PIND & (1 << ((n) + 5))) == 0)

#define DEBOUNCE_TIME 10

#define M_KEY_DOWN 8
#define M_KEY_UP 9
#define M_KEY_REPEAT 10

#define KEY_LEFT 0
#define KEY_MIDDLE 1
#define KEY_RIGHT 2

typedef enum {down, up} key_state;

void init_keys(void);

bool render_number(uint32_t number, uint8_t board[32]);

bool render_string(const char* string, uint8_t board[32]);

void read_string(char* str, uint8_t length, uint8_t timer);

#endif /* _IO_H */
