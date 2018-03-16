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


#include "matrix.h"
#include "Arduino.h"

uint8_t matrix_buffer[8];


void matrix_init(matrix_config_t* config)
{
	/* set pins */
  pinMode(config->data, OUTPUT);
  pinMode(config->clock, OUTPUT);
  pinMode(config->load, OUTPUT);

  /* create buffer */
  config->buffer = (uint8_t*) malloc(sizeof(uint8_t)*config->number*8);
  /* and init it */
  for (int i=0; i<config->number*8; i++)
    config->buffer[i] = 0;

	/* high */
  digitalWrite(config->clock, HIGH);

	   matrix_set_command(config, max7219_reg_scanLimit, 0x07);
	   matrix_set_command(config, max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
	   matrix_set_command(config, max7219_reg_shutdown, 0x01);    // not in shutdown mode
	   matrix_set_command(config, max7219_reg_displayTest, 0x00); // no display test

	/* empty registers, turn all LEDs off */
	matrix_clear(config);
	matrix_set_intensity(config, 0x0f);    // the first 0x0f is the value you can set
}

void matrix_shift_out(matrix_config_t* config, uint8_t data)
{
	uint8_t i;

	for (i=8; i > 0; i--)
	{
    digitalWrite(config->clock, LOW);
		uint8_t mask = 1 << (i-1);
		if (data & mask)
      digitalWrite(config->data, HIGH);
		else
      digitalWrite(config->data, LOW);

    digitalWrite(config->clock, HIGH);
	}
}

void matrix_set_intensity(matrix_config_t* config, uint8_t intensity)
{
  for (size_t i=0; i < config->number; i++)
  	matrix_set_command(config, max7219_reg_intensity, intensity);
}

void matrix_clear(matrix_config_t* config)
{
	uint8_t i;

	for (i=0; i<config->number*8; i++)
		matrix_set_column(config, i, 0);

	for (i=0; i<config->number*8; i++)
		config->buffer[i] = 0;
}

void matrix_set_command(matrix_config_t* config, uint8_t command, uint8_t value)
{
  digitalWrite(config->load, HIGH);

  for (int i = 0; i<config->number; i++)
  {
	   matrix_shift_out(config, command);
	   matrix_shift_out(config, value);
  }

  digitalWrite(config->load, LOW);
  digitalWrite(config->load, HIGH);
}

void matrix_set_column(matrix_config_t* config, uint8_t col, uint8_t value)
{
	uint8_t n = col / (8);
	uint8_t c = col % (8);

  digitalWrite(config->load, HIGH);

  for (int i=0; i<config->number-n-1; i++)
  {
    matrix_shift_out(config, 0);
    matrix_shift_out(config, 0);
  }

    matrix_shift_out(config, c + 1);
		matrix_shift_out(config, value);

    for (int i=0; i<n; i++) {
		  matrix_shift_out(config, 0);
		  matrix_shift_out(config, 0);
    }

  digitalWrite(config->load, LOW);
  digitalWrite(config->load, HIGH);

	matrix_buffer[col] = value;
}

void matrix_set_dot(matrix_config_t* config, uint8_t col, uint8_t row, uint8_t value)
{
	bit_write(config->buffer[col], row, value);

	uint8_t n = col / (8);
	uint8_t c = col % (8);
  digitalWrite(config->load, HIGH);

  for (int i=0; i<config->number-n-1; i++)
  {
    matrix_shift_out(config, 0);
    matrix_shift_out(config, 0);
  }
    matrix_shift_out(config, c + 1);
		matrix_shift_out(config, config->buffer[col]); //config->buffer[col]);

    for (int i=0; i<n; i++)
    {
		  matrix_shift_out(config, 0);
		  matrix_shift_out(config, 0);
    }

  digitalWrite(config->load, LOW);
  digitalWrite(config->load, HIGH);
}

void matrix_write_sprite(matrix_config_t* config, int x, int y, const uint8_t* sprite)
{
	uint8_t i, j;
	uint8_t w = sprite[0];
	uint8_t h = sprite[1];

	if (h == 8 && y == 0)
		for (i=0; i<w; i++)
		{
			int c = x + i;
			if (c>=0 && c<80)
				matrix_set_column(config, c, sprite[i+2]);
		}
	else
		for (i=0; i<w; i++)
			for (j=0; j<h; j++)
			{
				int c = x + i;
				int r = y + j;
				if (c>=0 && c<80 && r>=0 && r<8)
					matrix_set_dot(config, c, r, bit_read(sprite[i+2], j));
			}
}

void matrix_reload(matrix_config_t* config)
{
	uint8_t i;
	for (i=0; i<(config->number*8); i++)
	{
		uint8_t col = i;
    digitalWrite(config->load, HIGH);

		matrix_shift_out(config, i + 1);
		matrix_shift_out(config, matrix_buffer[col]);
		col += 8;

    digitalWrite(config->load, LOW);
    digitalWrite(config->load, HIGH);
	}
}

void matrix_shift_left(matrix_config_t* config, uint8_t rotate, uint8_t fill_zero)
{
	uint8_t old = matrix_buffer[0];
	uint8_t i;
	for (i=0; i<(config->number*8); i++)
		matrix_buffer[i] = matrix_buffer[i+1];
	if (rotate) matrix_buffer[(config->number*8)-1] = old;
	else if (fill_zero) matrix_buffer[(config->number*8)-1] = 0;

	matrix_reload(config);
}

void matrix_shift_right(matrix_config_t* config, uint8_t rotate, uint8_t fill_zero)
{
	uint8_t last = (config->number*8)-1;
	uint8_t old = matrix_buffer[last];
	uint8_t i;
	for (i=config->number*8; i>0; i--)
		matrix_buffer[i] = matrix_buffer[i-1];
	if (rotate) matrix_buffer[0] = old;
	else if (fill_zero) matrix_buffer[0] = 0;

	matrix_reload(config);
}

void matrix_shift_up(matrix_config_t* config, uint8_t rotate)
{
	uint8_t i;
	for (i=0; i<8; i++)
	{
		uint8_t b = matrix_buffer[i] & 1;
		matrix_buffer[i] >>= 1;
		if (rotate) bit_write(matrix_buffer[i], 7, b);
	}
	matrix_reload(config);
}

void matrix_shift_down(matrix_config_t* config, uint8_t rotate)
{
	uint8_t i;
	for (i=0; i<8; i++)
	{
		uint8_t b = matrix_buffer[i] & 128;
		matrix_buffer[i] <<= 1;
		if (rotate) bit_write(matrix_buffer[i], 0, b);
	}
	matrix_reload(config);
}
