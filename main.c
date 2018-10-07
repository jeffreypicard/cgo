/*
 * main.c
 *
 * Author: Jeffrey Picard
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct channel {
	int cap;
	int siz;
	int in_idx;
	int out_idx;
	void **data;
};

struct channel *
channel_resize(struct channel *c)
{
	void **to_free;
	int new_cap = c->cap * 2;
	void **new_data = calloc(new_cap, sizeof *new_data);

	if (!new_data) return 0;
	memcpy(new_data, c->data, (c->siz * sizeof *new_data));
	c->cap = new_cap;

	to_free = c->data;
	c->data = new_data;
	free(to_free);

	return c;
}

struct channel *
channel_make()
{
	int N = 2;
	struct channel *c = calloc(1, sizeof *c);

	if (!c) return 0;

	c->cap = N;
	c->data = calloc(N, sizeof *c->data);

	if (!c->data) return 0;

	return c;
}

void
channel_destroy(struct channel *c)
{
	if (!c) return;

	free(c->data);
	free(c);
}

int
channel_lock(struct channel *c)
{
	return 0;
}

int channel_unlock(struct channel *c)
{
	return 0;
}

int
channel_add(struct channel *c, void *d)
{
	if (channel_lock(c) != 0) return 1;

	if (c->siz + 1 >= c->cap) {
		if (!channel_resize(c)) return 1;
	}

	c->data[c->in_idx % c->cap] = d;
	c->siz++;

	if (channel_unlock(c) != 0) {
		c->siz--;
		return 1;
	}
	c->in_idx++;

	return 0;
}

void *
channel_get(struct channel *c)
{
	void *res;

	if (channel_lock(c) != 0) return 0;

	if (!c || c->siz <= 0) return 0;

	res = c->data[c->out_idx % c->cap];
	c->siz--;

	if (channel_unlock(c) != 0) {
		c->siz++;
		return 0;
	}
	c->out_idx++;

	return res;
}

int
main(int argc, char **argv)
{
	(void) argc;
	(void) argv;
	int x[10];
	int i;
	int res;
	void *res_p;

	for (i = 0; i < 10; i++) {
		x[i] = i;
	}

	struct channel *c = channel_make();

	for (i = 0; i < 10; i++) {
		channel_add(c, (void *) (x + i));
	}


	fprintf(stderr, "siz: %d\n", c->siz);

	for (i = 0; i < 10; i++) {
		res_p = channel_get(c);
		res = *(int *) res_p;
		fprintf(stderr, "%d\n", res);
	}

	channel_destroy(c);

	return 0;
}
