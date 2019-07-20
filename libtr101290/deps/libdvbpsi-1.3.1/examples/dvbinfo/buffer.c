/*****************************************************************************
 * buffer.c: buffer management
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

#include "config.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#if defined(HAVE_INTTYPES_H)
#   include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#   include <stdint.h>
#endif

#include <sys/types.h>
#include <assert.h>

typedef int64_t mtime_t;

#include "buffer.h"

struct fifo_s
{
    pthread_mutex_t lock;
    pthread_cond_t  wait;
    bool       b_force_wake;
    ssize_t    i_count;
    size_t     i_size; /* fifo size in bytes */
    buffer_t  *p_first;
    buffer_t **pp_last;
};

/* */
buffer_t *buffer_new(size_t i_size)
{
    buffer_t *buffer = (buffer_t*)malloc(sizeof(buffer_t) + i_size);
    if (buffer == NULL) return NULL;
    buffer->i_size = i_size;
    buffer->i_date = 0;
    buffer->p_next = NULL;
    buffer->p_data = (uint8_t*)((uint8_t *)buffer + sizeof(buffer_t));
    return buffer;
}

void buffer_free(buffer_t *buffer)
{
    free(buffer);
    buffer = NULL;
}

/* Fifo */
fifo_t *fifo_new(void)
{
    fifo_t *fifo = (fifo_t *) malloc(sizeof(fifo_t));
    if (fifo == NULL) return NULL;

    fifo->i_count = 0;
    fifo->i_size = 0;
    fifo->b_force_wake = false;
    fifo->p_first = NULL;
    fifo->pp_last = &fifo->p_first;
    pthread_mutex_init(&fifo->lock, NULL);
    pthread_cond_init(&fifo->wait, NULL);
    return fifo;
}

void fifo_free(fifo_t *fifo)
{
    if (fifo == NULL)
        return;

    pthread_mutex_lock(&fifo->lock);
    buffer_t *p = fifo->p_first;
    if (p != NULL)
    {
        fifo->i_count = 0;
        fifo->p_first = NULL;
        fifo->pp_last = &fifo->p_first;
    }
    pthread_mutex_unlock(&fifo->lock);

    while (p != NULL)
    {
        buffer_t *buffer;

        buffer = p->p_next;
        buffer_free(p);
        p = buffer;
    }

    pthread_cond_destroy(&fifo->wait);
    pthread_mutex_destroy(&fifo->lock);

    free(fifo);
    fifo = NULL;
}

void fifo_wake(fifo_t *fifo)
{
    pthread_mutex_lock(&fifo->lock);
    if (fifo->p_first == NULL)
        fifo->b_force_wake = true;
    pthread_cond_broadcast(&fifo->wait);
    pthread_mutex_unlock(&fifo->lock);
}

ssize_t fifo_count(fifo_t *fifo)
{
    pthread_mutex_lock(&fifo->lock);
    ssize_t count = fifo->i_count;
    pthread_mutex_unlock(&fifo->lock);
    return count;
}

size_t fifo_size(fifo_t *fifo)
{
    pthread_mutex_lock(&fifo->lock);
    size_t size = fifo->i_size;
    pthread_mutex_unlock(&fifo->lock);
    return size;
}

void fifo_push(fifo_t *fifo, buffer_t *buffer)
{
    buffer_t *p_last;
    size_t i_depth = 0;

    if (buffer == NULL)
        return;

    pthread_mutex_lock(&fifo->lock);
    for (p_last = buffer; ; p_last = p_last->p_next)
    {
         i_depth ++;
         if (!p_last->p_next)
             break;
    }

    *fifo->pp_last = buffer;
    fifo->pp_last = &p_last->p_next;
    fifo->i_count += i_depth;
    fifo->i_size += buffer->i_size;

    assert(fifo->p_first != NULL);
    assert(fifo->pp_last  != NULL);

    pthread_cond_signal(&fifo->wait);
    pthread_mutex_unlock(&fifo->lock);
}

buffer_t *fifo_pop(fifo_t *fifo)
{
    buffer_t *buffer;

    pthread_mutex_lock(&fifo->lock);

    while ((fifo->p_first == NULL) && !fifo->b_force_wake)
        pthread_cond_wait(&fifo->wait, &fifo->lock);

    buffer = fifo->p_first;
    fifo->b_force_wake = false;
    if (buffer == NULL)
    {
        pthread_mutex_unlock(&fifo->lock);
        return NULL;
    }

    fifo->p_first = buffer->p_next;
    fifo->i_count--;
    fifo->i_size -= buffer->i_size;

    if (fifo->p_first == NULL)
    {
        fifo->pp_last = &fifo->p_first;
    }
    pthread_mutex_unlock(&fifo->lock);

    buffer->p_next = NULL;
    return buffer;
}
