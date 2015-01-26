#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "mean_shift.h"

struct point * cluster = NULL;
struct point * first_point = NULL;
unsigned int kernel_radius = 0;

int add_cluster(unsigned int x, unsigned int y)
{
	static int nr_of_clusters = 0;

	struct point * current_cluster = cluster;
	struct point * last_cluster = NULL;
	while(current_cluster)
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
	new->next = NULL;
	new->x = x;
	new->y = y;
	new->cluster_nr = nr_of_clusters;
	return nr_of_clusters++;
}

void createFilter1(int width, double * matrix)
{
	double sigma = width/3.0;//		'apparently this is all you need to get a good approximation
	double norm = 1.0 / (sqrt(2*3.1415926) * sigma);//	'normalisation constant makes sure total of matrix is 1
	double coeff = 2*sigma*sigma;	//'the bit you divide x^2 by in the exponential
	double total = 0.0;
	for(int x = -width; x < width; x++ ) {
		double g = norm * exp( -x*x/coeff );
		matrix[x+width] = g;
		total += g;
	}
	for( int x=0 ;x < 2*width; x++) {//	'rescale things to get a total of 1, because of discretisation error
		matrix[x]/=total;
	}
}

void createFilter(int width, double * matrix)
{
	double sigma = width/2.0;
	double norm = 1.0;/// (sqrt(2*3.1415926) * sigma);//	'normalisation constant makes sure total of matrix is 1
	double coeff = 2*sigma*sigma;	//'the bit you divide x^2 by in the exponential
	double total = 0.0;
	for(int x = -width; x < width; x++ ) {
		double g = norm * exp( -(x*x/coeff));
		matrix[x+width] = g;
		total += g;
	}
	for( int x=0 ;x < 2*width; x++) {//	'rescale things to get a total of 1, because of discretisation error
		matrix[x]/=total;
	}
}

void apply_kernel(double x, double y, double x1, double y1, double *shift_x, double *shift_y)
{
	static double * matrix = NULL;
	int matrix_size = 1000;
	if(!matrix) {
		matrix = (double *) malloc((matrix_size*2+1)*sizeof(double));
		createFilter(matrix_size, matrix);
	}
	double x_d = x1 - x;
	double y_d = y1 - y;
	if(fabs(x_d) >= kernel_radius || fabs(y_d) >= kernel_radius) return;
	*shift_x += x_d*matrix[(int)(((x_d+kernel_radius)*matrix_size-1)/kernel_radius)];
	*shift_y += y_d*matrix[(int)(((y_d+kernel_radius)*matrix_size-1)/kernel_radius)];
}

unsigned int do_mean_shift(double x, double y)
{
	double shift_x = 0.0;
	double shift_y = 0.0;
	struct point * current_point = first_point;
	while(current_point)
	{
		apply_kernel(x, y, current_point->x, current_point->y, &shift_x, &shift_y);
		current_point = current_point->next;
	}
	//printf("x: %f, y: %f, shift_x: %f, shift_y: %f\n", x, y, shift_x, shift_y);
	//else shift_x /= point_count;
	//else shift_y /= point_count;
	if(abs(shift_x) == 0.0 && abs(shift_y) == 0.0)
		return add_cluster(x,y);
	return do_mean_shift(x + shift_x, y + shift_y);
}

void mean_shift(struct point * start_point, unsigned int kernel_size)
{
	kernel_radius = 50000;//kernel_size;
	first_point = start_point;
	struct point * current_point = first_point;
	while(current_point)
	{
		//printf("x is: %d, y is %d\n", current_point->x, current_point->y);
		current_point->cluster_nr = do_mean_shift(current_point->x, current_point->y);
		printf("%i\n", current_point->cluster_nr);
		fflush(stdout);
		current_point = current_point->next;
	}
}
