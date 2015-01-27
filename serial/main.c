#include <stdio.h>
#include <stdlib.h>
#include "mean_shift.h"

int main(int argc, char *argv[])
{
	if(argc < 2 && argc > 3) {
		printf( "usage: %s <image.txt> <kernel_size\n", argv[0]);
		return -1;
	}

	FILE *file = fopen(argv[1], "r");
	if(file == 0) {
		printf( "Could not open file\n" );
		return -1;
	}
	int kernel_size = atoi(argv[2]);

	unsigned int x, y;
	struct point * first_point = (struct point *) malloc(sizeof(struct point));
	struct point * current_point = first_point;
	struct point * prev_point = current_point;
	while(1) {
		if(fscanf(file, "%d %d", &x, &y) < 2)
			break;
		current_point->x = x;
		current_point->y = y;
		current_point->next = (struct point *) malloc(sizeof(struct point));
		prev_point = current_point;
		current_point = prev_point->next;
	}
	prev_point->next = NULL;
	free(current_point);
	fclose(file);

	mean_shift(first_point, kernel_size);

	return 0;
}
