#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include <stdint.h>

bool render_number(uint32_t number, uint8_t board[32]);

bool render_string(const char* string, uint8_t board[32]);

#endif /* IO_H */
