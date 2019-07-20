/*****************************************************************************
 * buffer.h: buffer management
 *****************************************************************************
 * Copyright (C) 2011 M2X BV
 *
 * Authors: Jean-Paul Saman <jpsaman@videolan.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#ifndef DVBINFO_BUFFER_H_
#define DVBINFO_BUFFER_H_

typedef struct buffer_s buffer_t;

struct buffer_s
{
    size_t   i_size;    /* size of buffer data */
    mtime_t  i_date;    /* timestamp */
    buffer_t *p_next;   /* pointer to next buffer_t */
    uint8_t  *p_data;   /* actuall buffer data */
};

typedef struct fifo_s fifo_t;

/* Buffer management:
 * buffer_new()  - create new buffer of size i_size + plus header structure
 * buffer_free() - free buffer
 */
buffer_t *buffer_new(size_t i_size);
void buffer_free(buffer_t *buffer);

/* Fifo:
 * fifo_new()  - create a new fifo holding buffer_t pointers
 * fifo_free() - release fifo and all buffers contained therein
 * fifo_count()- number of buffers in fifo_t
 * fifo_size() - total size of buffers in fifo_t
 * fifo_push() - push buffer at end of fifo
 * fifo_pop()  - pop buffer from start of fifo
 * fifo_wake() - wake up fifo listeners
 */
fifo_t *fifo_new(void);
void fifo_free(fifo_t *fifo);
ssize_t fifo_count(fifo_t *fifo);
size_t fifo_size(fifo_t *fifo);
void fifo_push(fifo_t *fifo, buffer_t *buffer);
buffer_t *fifo_pop(fifo_t *fifo);
void fifo_wake(fifo_t *fifo);

#endif
