/*
 * main.c
 *
 * Author: Jeffrey Picard
 */

#define _DEFAULT_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>
#include <time.h>
//#include <sys/time.h>


struct channel {
	uint32_t	cap;
	uint32_t	siz;
	uint32_t	in_idx;
	uint32_t	out_idx;
	pthread_mutex_t mut;
	void 		**data;
};

struct c_routine {
	pthread_t 	t;
	void 		*(*func)(void *);
	uint64_t        *args;
	int 		num_args;
};

// Assembly function
void *cgo_asm(void *);
int atomic_insert(void **old_val, void *new_val);

void
free_c_routine(struct c_routine *c)
{
	//pthread_cancel(c->t);
	free(c->args);
	free(c);
}

struct c_routine *
cgo(int num_args, ...)
{
	va_list ap;
	int i;
	int num_args_to_func;
	struct c_routine *c_rout;
	uint64_t *args;
	void *arg;

	num_args_to_func = num_args + 2;
	args = calloc(num_args_to_func, sizeof *args);

	if (!args) return NULL;

	c_rout = calloc(1, sizeof *c_rout);

	if (!c_rout) {
		free(args);
		return NULL;
	}

	args[0] = num_args_to_func;
	va_start(ap, num_args);
	for(i = 1; i < num_args_to_func; i++) {
		arg = va_arg(ap, void *);
		args[i] = *(uint64_t*) &arg;
	}
	va_end(ap);

	c_rout->func = (void * (*)(void *)) args[0];
	c_rout->args = args;
	c_rout->num_args = num_args_to_func;

	//fprintf(stderr, "cgo_asm: %p\n", (void *)cgo_asm);
	if (pthread_create(&c_rout->t, 0, cgo_asm, args) != 0) {
		free_c_routine(c_rout);
		return NULL;
	}

	return c_rout;
}



struct channel *
channel_resize(struct channel *c)
{
	void **to_free;
	int new_cap = c->cap * 2;
	void **new_data = calloc(new_cap, sizeof *new_data);

	if (!new_data) return NULL;
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
	int N = 512;
	struct channel *c = calloc(1, sizeof *c);

	if (!c) return NULL;

	c->cap = N;
	c->data = calloc(N, sizeof *c->data);

	if (!c->data) return NULL;

	pthread_mutex_init(&c->mut, 0);

	return c;
}

void
channel_destroy(struct channel *c)
{
	if (!c) return;

	pthread_mutex_destroy(&c->mut);
	free(c->data);
	free(c);
}

/*
 * FIXME
 */
int
channel_lock(struct channel *c)
{
	pthread_mutex_lock(&c->mut);
	return 0;
}

/*
 * FIXME
 */
int
channel_unlock(struct channel *c)
{
	pthread_mutex_unlock(&c->mut);
	return 0;
}

int
channel_add(struct channel *c, void *d)
{
	struct timespec rmtp;
	rmtp.tv_sec = 0;
	rmtp.tv_nsec = 1000;

	while (channel_lock(c) != 0) {sleep(1);}

	if (c->siz - 1 >= c->cap) {
		if (!channel_resize(c)) return 1;
	}

	int i = c->in_idx % c->cap;
	c->data[i] = d;
	/*
	while(!atomic_insert(&c->data[i], d)) {
		nanosleep(&rmtp, NULL);
	}
	while(!atomic_insert(&c->data[i], d)){}
	*/	


	c->siz++;
	c->in_idx++;

	if (channel_unlock(c) != 0) {
		//c->siz--;
		//Should never happen?
		return 1;
	}

	return 0;
}

void *
channel_get(struct channel *c)
{
	void *res;

	if (channel_lock(c) != 0) return NULL;

	if (!c) return NULL;

	/*
	 * FIXME
	 * Probably very inefficient, but something like this is needed
	 * for a channel read to block until there's a message ready
	 */
	while (c->siz == 0) {
		channel_unlock(c);
		//sleep(1); // Tune this FIXME
		channel_lock(c);
	}

	res = c->data[c->out_idx % c->cap];
	c->siz--;

	if (channel_unlock(c) != 0) {
		c->siz++;
		return NULL;
	}
	c->out_idx++;

	return res;
}

int
par_sum(uint64_t *x, uint64_t n, struct channel *c)
{
	uint64_t i;
	uint64_t sum;

	sum = 0;
	for (i = 0; i < n; i++) {
		sum += x[i];
	}
	channel_add(c, (void *) sum);

	return 0;
}

int
ret_n(int n)
{
	return n;
}

int
tests()
{
	long N = 2 << 10;
	long n1, n2;
	long *x, *x1, *x2;
	long i, j, k;
	//int part1, part2;
	struct channel *c;
	struct timespec beg, end;
	int splits = 4;
	uint64_t sum = 0, real_sum = 0;

	srandom(time(NULL));
	x = calloc(N, sizeof *x1);

	for (i = 0; i < N; i++) {
		x[i] = random() % 100;
		real_sum += x[i];
	}

	/*
	long r = random() % N;
	n1 = r;
	n2 = N - r;
	x2 = x1 + r;

	struct c_routine *crt;
	struct c_routine *crt2;

	cgo(3, par_sum, x1, n1, c);
	cgo(3, par_sum, x2, n2, c);
	*/

	int part_siz = N / splits;

	c = channel_make();

	clock_gettime(CLOCK_REALTIME, &beg);


	i = 0;
	j = 0;
	while (i < N) {
		j++;
		cgo(3, par_sum, x, part_siz, c);

		i += part_siz;
		x += part_siz;

		if (i + part_siz > N) {
			part_siz = N - i;
		}
	}

	i = 0;
	uint64_t int_sum;
	while (i < j) {
		i++;
		int_sum = (long) channel_get(c);
		sum += int_sum;
	}

	clock_gettime(CLOCK_REALTIME, &end);


	fprintf(stderr,
		"N: %ld, real_sum: %ld, sum: %ld, time: %ld / %ld\n",
		N, real_sum, sum,
		end.tv_sec - beg.tv_sec,
		end.tv_nsec - beg.tv_nsec);

	/*
	 * Don't need this for testing the OS just reclaims it anyways
	void *res, *res2;
	pthread_join(crt->t, &res);
	pthread_join(crt2->t, &res2);
	fprintf(stderr, "%p, %p\n", res, res2);


	free(x1);
	channel_destroy(c);
	free_c_routine(crt);
	free_c_routine(crt2);

	fprintf(stderr, "%d, %d, %d\n", part1, part2, part1 + part2);
	*/

	return 0;
}

int
main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	tests();

	return 0;
}
