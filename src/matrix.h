/*
   Copyright (c) 2014 - David Pello

   This file is part of BikeLight.

   BikeLight is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BikeLight is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with BileLight.  If not, see <http://www.gnu.org/licenses/>.

*/


#include <inttypes.h>

/* config */
//#define MATRIX_DIR              P2DIR
//#define MATRIX_OUT              P2OUT
//#define MATRIX_DATA_PIN         BIT0
//#define MATRIX_LOAD_PIN         BIT1
//#define MATRIX_CLOCK_PIN        BIT2

struct matrix_config
{
  uint8_t data;
  uint8_t load;
  uint8_t clock;
  uint8_t number;
  uint8_t *buffer;
};

typedef struct matrix_config matrix_config_t;

/* registers */
#define max7219_reg_noop        0x00
#define max7219_reg_digit0      0x01
#define max7219_reg_digit1      0x02
#define max7219_reg_digit2      0x03
#define max7219_reg_digit3      0x04
#define max7219_reg_digit4      0x05
#define max7219_reg_digit5      0x06
#define max7219_reg_digit6      0x07
#define max7219_reg_digit7      0x08
#define max7219_reg_decodeMode  0x09
#define max7219_reg_intensity   0x0a
#define max7219_reg_scanLimit   0x0b
#define max7219_reg_shutdown    0x0c
#define max7219_reg_displayTest 0x0f

#define bit_read(value, bit) (((value) >> (bit)) & 0x01)
#define bit_set(value, bit) ((value) |= (1UL << (bit)))
#define bit_clear(value, bit) ((value) &= ~(1UL << (bit)))
#define bit_write(value, bit, bitvalue) (bitvalue ? bit_set(value, bit) : bit_clear(value, bit))


/* interface */

void matrix_reload(matrix_config_t* config);
void matrix_shift_out(matrix_config_t* config, uint8_t data);
void matrix_init(matrix_config_t* config);
void matrix_clear(matrix_config_t* config);
void matrix_set_command(matrix_config_t* config, uint8_t command, uint8_t value);
void matrix_set_intensity(matrix_config_t* config, uint8_t intensity);
void matrix_set_column(matrix_config_t* config, uint8_t col, uint8_t value);
void matrix_set_dot(matrix_config_t* config, uint8_t col, uint8_t row, uint8_t value);
void matrix_write_sprite(matrix_config_t* config, int x, int y, const uint8_t* sprite);

void matrix_shift_left(matrix_config_t* config, uint8_t rotate, uint8_t fill_zero);
void matrix_shift_right(matrix_config_t* config, uint8_t rotate, uint8_t fill_zero);
void matrix_shift_up(matrix_config_t* config, uint8_t rotate);
void matrix_shift_down(matrix_config_t* config, uint8_t rotate);
