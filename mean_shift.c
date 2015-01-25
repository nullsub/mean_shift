#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "mean_shift.h"

struct point * cluster = NULL;
struct point * first_point = NULL;

int add_cluster(unsigned int x, unsigned int y)
{
	static int nr_of_clusters = 0;	

	struct point * current_cluster = cluster;
	struct point * last_cluster = NULL;
	while(current_cluster != NULL)
	{
		if(current_cluster->x == x && current_cluster->y == y) {
			return current_cluster->cluster_nr;
		}
		last_cluster = current_cluster;
		current_cluster = current_cluster->next;
	}

	struct point * new = (struct point *) malloc(sizeof(struct point));
	if(last_cluster) last_cluster->next = new;
	else cluster = new;
	new->x = x;
	new->y = y;
	new->cluster_nr = nr_of_clusters;
	return nr_of_clusters++;
}

double kernel_x( double x, double y, double x1, double y1)
{
	double distance = x1 - x;
	return distance;
}

double kernel_y( double x, double y, double x1, double y1)
{
	double distance = y1 - y;
	return distance;
}

unsigned int do_mean_shift(double x, double y)
{
	double shift_x = 0;
	double shift_y = 0;
	int point_count = 0;
	struct point * current_point = first_point;
	while(current_point)
	{
		point_count ++;
		shift_x += kernel_x(x, y, current_point->x, current_point->y);
		shift_y += kernel_y(x, y, current_point->x, current_point->y);
		current_point = current_point->next;
	}
	shift_x /= point_count;
	shift_y /= point_count;
//	printf("x: %f, y: %f, shift_x: %f, shift_y: %f\n", x, y, shift_x, shift_y);
	if(abs(shift_x) < 1.0) shift_x = 0;
	if(abs(shift_y) < 1.0) shift_y = 0;
	if(shift_x == 0 && shift_y == 0)
		return add_cluster(x,y);
	return do_mean_shift(x + shift_x, y + shift_y);
}

void mean_shift(struct point * start_point, unsigned int kernel_size)
{
	first_point = start_point;
	struct point * current_point = first_point;
	while(current_point)
	{
	//	printf("x is: %d, y is %d\n", current_point->x, current_point->y);
		current_point->cluster_nr = do_mean_shift(current_point->x, current_point->y);
		printf("%i\n", current_point->cluster_nr);
		fflush(stdout);
		current_point = current_point->next;
	}
}
