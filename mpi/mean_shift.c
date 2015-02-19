#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include "mean_shift.h"

#define FLOAT_DELTA 0.001

struct point * first_cluster = NULL;
struct point * first_point = NULL;
unsigned int kernel_radius = 0;

int add_cluster(int x, int y)
{
	static int nr_of_clusters = 0;

	struct point * current_cluster = first_cluster;
	struct point * last_cluster = NULL;
	while(current_cluster) {
		if(abs(current_cluster->x - x) < 100 && abs(current_cluster->y - y) < 100)
			return current_cluster->cluster_nr;
		last_cluster = current_cluster;
		current_cluster = current_cluster->next;
	}

	struct point * new = (struct point *) malloc(sizeof(struct point));
	if(last_cluster) last_cluster->next = new;
	else first_cluster = new;
	new->next = NULL;
	new->x = x;
	new->y = y;
	new->cluster_nr = nr_of_clusters;
	return nr_of_clusters++;
}

void create_filter(int width, double * matrix)
{
	double sigma = width/3.0;
	double norm = 1.0/(sqrt(2*M_PI) * sigma);
	double sum = 0.0;
	for(int x = -width; x < width; x++) {
		double g = norm * exp(-((x*x)/(2*sigma*sigma)));
		matrix[x+width] = g;
		sum += g;
	}
	for(int x = 0; x < width*2; x++) {
		matrix[x] /= sum*(kernel_radius/width); //normalize
	}
}

void apply_kernel(double x, double y, double x1, double y1, double *shift_x, double *shift_y)
{
	unsigned const int matrix_size = 100;
	static double * matrix = NULL;
	if(!matrix) {
		matrix = (double *) malloc((matrix_size*2)*sizeof(double));
		create_filter(matrix_size, matrix);
	}
	double x_d = x1 - x;
	double y_d = y1 - y;
	if(fabs(x_d) < FLOAT_DELTA && fabs(y_d) <  FLOAT_DELTA) return;
	if((int)fabs(x_d) >= kernel_radius || (int)fabs(y_d) >= kernel_radius) return;

	*shift_x += x_d*matrix[(int)(((x_d*(double)(matrix_size))/(double)kernel_radius))+matrix_size];
	*shift_y += y_d*matrix[(int)(((y_d*(double)(matrix_size))/(double)kernel_radius))+matrix_size];
}

unsigned int do_mean_shift(double x, double y)
{
	while(1) {
		double shift_x = 0.0;
		double shift_y = 0.0;
		struct point * current_point = first_point;
		while(current_point) {
			apply_kernel(x, y, current_point->x, current_point->y, &shift_x, &shift_y);
			current_point = current_point->next;
		}
		//printf("shift_x: %f, shift_y: %f\n", shift_x, shift_y);
		x += shift_x;
		y += shift_y;
		if(fabs(shift_x) < FLOAT_DELTA && fabs(shift_y) < FLOAT_DELTA)
			return add_cluster(x, y);
	}
}

void mean_shift(struct point * start_point, unsigned int kernel_size)
{
	kernel_radius = kernel_size;
	first_point = start_point;

	int numprocs, rank, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
        MPI_Status status;

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(processor_name, &namelen);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	unsigned int point_cntr = 0;
	struct point * current_point = first_point;
	while(current_point) {
		if(point_cntr % numprocs == rank)
			current_point->cluster_nr = do_mean_shift(current_point->x, current_point->y);
		current_point = current_point->next;
		point_cntr++;
	}
	//printf("rank %i, finished analyzing numranks is %i\n", rank, numprocs);
	fflush(stdout);
	const int CLUSTER_SIZE_TAG = 2;
	const int CLUSTER_ARRAY_TAG = 3;
	const int POINTER_ARRAY_TAG = 5;
	if(rank == 0) {
		int translate_ids[numprocs-1][(point_cntr/numprocs)*3];
		unsigned int point_cluster_id[numprocs-1][point_cntr/numprocs];
		for(int i = 0; i < numprocs-1; i++) {
			//merge clusters
			int nr_clusters;
			MPI_Recv(&nr_clusters, 1, MPI_INT, i+1, CLUSTER_SIZE_TAG, MPI_COMM_WORLD, &status);

			int receive_clusters[nr_clusters];
			MPI_Recv(&receive_clusters, nr_clusters, MPI_INT, i+1, CLUSTER_ARRAY_TAG, MPI_COMM_WORLD, &status);

			for(int j = 0; j < nr_clusters; j += 3) {
				translate_ids[i][receive_clusters[j]] = add_cluster(receive_clusters[j+1], receive_clusters[j+2]);
			}
			//get cluster id for every point
			MPI_Recv(&point_cluster_id[i], point_cntr/numprocs, MPI_INT, i+1, POINTER_ARRAY_TAG, MPI_COMM_WORLD, &status);
		}
		struct point * current_point = first_point;
		unsigned int point_cntr = 0;
		while(current_point) {
			if(point_cntr % numprocs == 0)
				printf("%i\n", current_point->cluster_nr);
			else
				printf("%i\n", translate_ids[(point_cntr% numprocs)-1][point_cluster_id[(point_cntr % numprocs)-1][point_cntr/numprocs]]);
			fflush(stdout);
			current_point = current_point->next;
			point_cntr ++;
		}
	}
	else {
		// clusters
		int translate_ids[(point_cntr/numprocs)*3];
		int cluster_cntr = 0;
		struct point * current_cluster = first_cluster;
		while(current_cluster) {
			translate_ids[cluster_cntr++] = current_cluster->cluster_nr;
			translate_ids[cluster_cntr++] = current_cluster->x;
			translate_ids[cluster_cntr++] = current_cluster->y;
			current_cluster = current_cluster->next;
		}
		MPI_Send(&cluster_cntr, 1, MPI_INT, 0, CLUSTER_SIZE_TAG, MPI_COMM_WORLD);
		MPI_Send(&translate_ids, cluster_cntr, MPI_INT, 0, CLUSTER_ARRAY_TAG, MPI_COMM_WORLD);

		//send which point belongs to which cluster
		unsigned int point_cluster_id[point_cntr/numprocs];
		struct point * current_point = first_point;
		unsigned int point_cntr = 0;
		while(current_point) {
			if(point_cntr % numprocs == rank) {
				point_cluster_id[point_cntr/numprocs] = current_point->cluster_nr;
			}
			current_point = current_point->next;
			point_cntr++;
		}
		MPI_Send(&point_cluster_id, point_cntr/numprocs, MPI_INT, 0, POINTER_ARRAY_TAG, MPI_COMM_WORLD);
	}
	MPI_Finalize();
	fflush(stdout);
}
