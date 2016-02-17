#include <stdlib.h>
#include "io.h"
#include "numbers.h"
#include "letters.h"
#include "ht1632c.h"

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
