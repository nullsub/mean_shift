#include <stdio.h>
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
	fclose(file);
	return 0;
}
