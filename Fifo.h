/*
 * Fifo.h
 *
 * Copyright (c) 2018 Yulay Rakhmangulov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CDCHANGER_FIFO_H
#define CDCHANGER_FIFO_H

#include <stdint.h>

enum
{
    MAX_FIFO_BUF_SIZE = 32
};

struct Fifo
{
  uint8_t buf[MAX_FIFO_BUF_SIZE];
  uint8_t head;
  uint8_t tail;
};


#endif //CDCHANGER_FIFO_H
