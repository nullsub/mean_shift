#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "mean_shift.h"

int main(int argc, char *argv[])
{
	if ( argc != 2 ) {
		printf( "usage: %s <image.txt>\n", argv[0]);
		return -1;
	}

	FILE *file = fopen( argv[1], "r" );
	if (file == 0) {
		printf( "Could not open file\n" );
		return -1;
	}

	unsigned int x, y;
	struct point * first_point = (struct point *) malloc(sizeof(struct point));
	struct point * current_point = first_point;
	struct point * prev_point = NULL;
	while(1) {
		if(fscanf(file, "%d %d", &x, &y) < 2)	
			break;
		current_point->x = x;
		current_point->y = y;
		if(prev_point)
			prev_point->next = current_point;
		prev_point = current_point;
		current_point = (struct point *) malloc(sizeof(struct point));
	}
	prev_point->next = NULL;

	mean_shift(first_point, 2000);

	fclose(file);
	return 0;
}
