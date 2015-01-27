#ifndef MEAN_SHIFT_H
#define MEAN_SHIFT_H

struct point{
	unsigned int x;
	unsigned int y;
	unsigned int cluster_nr;
	void * next;
};

void mean_shift(struct point * start_point, unsigned int kernel_size);

#endif
