/* https://stackoverflow.com/questions/215557/how-do-i-implement-a-circular-list-ring-buffer-in-c */
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t fifo_data_t;

typedef struct {
	size_t head;
	size_t tail;
	size_t size;
	uint8_t *buffer;
} fifo_t;

fifo_t *fifo_new(size_t size);
size_t fifo_push(fifo_t * fifo, fifo_data_t data);
size_t fifo_push_block(fifo_t * fifo, fifo_data_t * block, size_t count);
size_t fifo_pop(fifo_t * fifo, fifo_data_t * data);
size_t fifo_pop_block(fifo_t * fifo, fifo_data_t * block, size_t count);
bool fifo_is_full(fifo_t * fifo);
bool fifo_is_empty(fifo_t * fifo);
size_t fifo_count(fifo_t * fifo);

fifo_t *fifo_new(size_t size)
{
	fifo_data_t *buffer = malloc(size * sizeof(buffer[0]));

	if (!buffer)
		return NULL;

	fifo_t *fifo = malloc(sizeof(fifo_t));

	if (!fifo) {
		free(buffer);
		return NULL;
	}

	fifo->buffer = buffer;
	fifo->head = 0;
	fifo->tail = 0;
	fifo->size = size;

	return fifo;
}

void fifo_free(fifo_t *fifo)
{
	if(!fifo)
		return;
	free(fifo->buffer);
	free(fifo);
}

size_t fifo_push(fifo_t * fifo, fifo_data_t data)
{
	assert(fifo);

	if (fifo_is_full(fifo))
		return 0;

	fifo->buffer[fifo->head] = data;

	fifo->head++;
	if (fifo->head == fifo->size)
		fifo->head = 0;

	return 1;
}

size_t fifo_push_block(fifo_t * fifo, fifo_data_t * block, size_t count)
{
	assert(fifo);
	assert(block);

	for (size_t i = 0; i < count; i++)
		if (fifo_push(fifo, block[i]) == 0)
			return i;
	return count;
}

size_t fifo_pop(fifo_t * fifo, fifo_data_t * data)
{
	assert(fifo);
	assert(data);

	if (fifo_is_empty(fifo))
		return 0;

	*data = fifo->buffer[fifo->tail];

	fifo->tail++;
	if (fifo->tail == fifo->size)
		fifo->tail = 0;

	return 1;
}

size_t fifo_pop_block(fifo_t * fifo, fifo_data_t * block, size_t count)
{
	assert(fifo);
	assert(block);

	for (size_t i = 0; i < count; i++)
		if (fifo_pop(fifo, block + i) == 0)
			return i;

	return count;
}

bool fifo_is_full(fifo_t * fifo)
{
	assert(fifo);
	return (fifo->head == (fifo->size - 1) && fifo->tail == 0)
	    || (fifo->head == (fifo->tail - 1));
}

bool fifo_is_empty(fifo_t * fifo)
{
	assert(fifo);
	return fifo->head == fifo->tail;
}

size_t fifo_count(fifo_t * fifo)
{
	assert(fifo);
	if (fifo_is_empty(fifo))
		return 0;
	else if (fifo_is_full(fifo))
		return fifo->size;
	else if (fifo->head < fifo->tail)
		return (fifo->head) + (fifo->size - fifo->tail);
	else
		return fifo->head - fifo->tail;
}

#ifdef USE_MAIN
int main(void)
{
	fifo_t *f = fifo_new(16);

	for(int i = 0; i < 32; i++) {
		printf("%d: push %d %s\n", i, i, fifo_push(f, i) ? "ok" : "full");
	}


	for(int i = 0; i < 32; i++) {
		fifo_data_t d = 0;
		size_t r = fifo_pop(f, &d);
		printf("%d: pop %d %s\n", i, (int)d, r ? "ok" : "empty");
	}


	fifo_free(f);
	return 0;
}
#endif
