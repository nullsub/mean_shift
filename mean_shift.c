#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "mean_shift.h"

struct point * cluster = NULL;
struct point * first_point = NULL;
unsigned int kernel_radius = 0;

int add_cluster(int x, int y)
{
	static int nr_of_clusters = 0;

	struct point * current_cluster = cluster;
	struct point * last_cluster = NULL;
	while(current_cluster) {
		if(abs(current_cluster->x - x) < 10 && abs(current_cluster->y - y) < 10)
			return current_cluster->cluster_nr;
		last_cluster = current_cluster;
		current_cluster = current_cluster->next;
	}

	struct point * new = (struct point *) malloc(sizeof(struct point));
	if(last_cluster) last_cluster->next = new;
	else cluster = new;
	new->next = NULL;
	new->x = x;
	new->y = y;
	new->cluster_nr = nr_of_clusters;
	return nr_of_clusters++;
}

void create_filter(int width, double * matrix)
{
	double sigma = width/3.0;
	double norm = 1.0/(sqrt(2*3.1415926) * sigma);
	double total = 0.0;
	for(int x = -width; x < width; x++ ) {
		double g = norm * exp(-((x*x)/(2*sigma*sigma)));
		matrix[x+width] = g;
		total += g;
	}
	for( int x=0 ;x < 2*width; x++) {
		matrix[x]/=total*(kernel_radius/width); //normalize
	}
}

void apply_kernel(double x, double y, double x1, double y1, double *shift_x, double *shift_y)
{
	unsigned const int matrix_size = 100;
	static double * matrix = NULL;
	if(!matrix) {
		matrix = (double *) malloc((matrix_size*2+1)*sizeof(double));
		create_filter(matrix_size, matrix);
	}
	double x_d = x1 - x;
	double y_d = y1 - y;
	if(fabs(x_d) < 0.1 && fabs(y_d) <  0.1) return;
	if((int)fabs(x_d) >= kernel_radius || (int)fabs(y_d) >= kernel_radius) return;

	*shift_x += x_d*matrix[(int)(((x_d*(double)(matrix_size))/(double)kernel_radius))+matrix_size];
	*shift_y += y_d*matrix[(int)(((y_d*(double)(matrix_size))/(double)kernel_radius))+matrix_size];
}

unsigned int do_mean_shift(double x, double y)
{
	double shift_x = 0.0;
	double shift_y = 0.0;
	struct point * current_point = first_point;
	while(current_point) {
		apply_kernel(x, y, current_point->x, current_point->y, &shift_x, &shift_y);
		current_point = current_point->next;
	}
	if(fabs(shift_x) < 0.001 && fabs(shift_y) < 0.001)
		return add_cluster(x, y);
	return do_mean_shift(x + shift_x, y + shift_y);
}

void mean_shift(struct point * start_point, unsigned int kernel_size)
{
	kernel_radius = kernel_size;
	first_point = start_point;
	struct point * current_point = first_point;
	while(current_point) {
		current_point->cluster_nr = do_mean_shift(current_point->x, current_point->y);
		printf("%i\n", current_point->cluster_nr);
		fflush(stdout);
		current_point = current_point->next;
	}
}
