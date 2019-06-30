#include "MarchingCubes.h"

#define MARCHING_CUBES_LOOKUP_TABLE_SIZE 4096

const int32 MARCHING_CUBES_LOOKUP_TABLE[4096] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 ,
	3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 ,
	3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 ,
	3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 ,
	9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 ,
	9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 ,
	2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 ,
	8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 ,
	9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 ,
	4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 ,
	3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 ,
	1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 ,
	4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 ,
	4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 ,
	9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 ,
	5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 ,
	2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 ,
	9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 ,
	0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 ,
	2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 ,
	10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 ,
	4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 ,
	5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 ,
	5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 ,
	9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 ,
	0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 ,
	1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 ,
	10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 ,
	8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 ,
	2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 ,
	7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 ,
	9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 ,
	2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 ,
	11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 ,
	9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 ,
	5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 ,
	11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 ,
	11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 ,
	1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 ,
	9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 ,
	5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 ,
	2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 ,
	0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 ,
	5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 ,
	6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 ,
	3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 ,
	6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 ,
	5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 ,
	1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 ,
	10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 ,
	6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 ,
	8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 ,
	7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 ,
	3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 ,
	5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 ,
	0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 ,
	9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 ,
	8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 ,
	5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 ,
	0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 ,
	6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 ,
	10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 ,
	10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 ,
	8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 ,
	1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 ,
	3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 ,
	0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 ,
	10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 ,
	3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 ,
	6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 ,
	9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 ,
	8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 ,
	3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 ,
	6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 ,
	0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 ,
	10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 ,
	10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 ,
	2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 ,
	7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 ,
	7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 ,
	2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 ,
	1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 ,
	11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 ,
	8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 ,
	0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 ,
	7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 ,
	10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 ,
	2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 ,
	6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 ,
	7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 ,
	2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 ,
	1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 ,
	10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 ,
	10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 ,
	0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 ,
	7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 ,
	6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 ,
	8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 ,
	9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 ,
	6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 ,
	4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 ,
	10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 ,
	8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 ,
	0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 ,
	1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 ,
	8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 ,
	10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 ,
	4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 ,
	10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 ,
	5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 ,
	11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 ,
	9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 ,
	6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 ,
	7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 ,
	3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 ,
	7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 ,
	9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 ,
	3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 ,
	6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 ,
	9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 ,
	1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 ,
	4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 ,
	7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 ,
	6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 ,
	3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 ,
	0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 ,
	6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 ,
	0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 ,
	11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 ,
	6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 ,
	5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 ,
	9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 ,
	1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 ,
	1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 ,
	10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 ,
	0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 ,
	5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 ,
	10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 ,
	11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 ,
	9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 ,
	7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 ,
	2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 ,
	8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 ,
	9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 ,
	9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 ,
	1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 ,
	9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 ,
	9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 ,
	5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 ,
	0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 ,
	10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 ,
	2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 ,
	0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 ,
	0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 ,
	9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 ,
	5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 ,
	3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 ,
	5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 ,
	8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 ,
	9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 ,
	0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 ,
	1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 ,
	3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 ,
	4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 ,
	9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 ,
	11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 ,
	11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 ,
	2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 ,
	9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 ,
	3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 ,
	1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 ,
	4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 ,
	4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 ,
	0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 ,
	3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 ,
	3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 ,
	0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 ,
	9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 ,
	1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 ,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 
};

struct NoiseFunction
{
	uint32 width;
	uint32 height;
	uint32 depth;
	cmg::noise::PerlinNoise<float> noise;

	Vector4f GetPoint(uint32 x, uint32 y, uint32 z)
	{
		Vector4f v;
		v.xyz = Vector3f(
			(float) x - width*0.5f,
			(float) y - height*0.5f,
			(float) z - depth*0.5f);
		v.w = (float) z - depth*0.5f;
		v.w += Math::Sin((float) x * 0.7f);
		v.w += Math::Sin((float) y * 0.3f) * 1.5f;
		Vector3f n = Vector3f(
			(float) x / width,
			(float) y / height,
			(float) z / depth);
		n = Vector3f((float) x, (float) y, (float) z) / 32.0f;

		float warp = noise.Noise(n);
		n.x += warp * 1;

		float feature = depth * 0.2f;
		float density = (float) z - depth * 0.5f;
		density += noise.Noise(n * 4.03f) * 0.25f * feature;
		density += noise.Noise(n * 1.96f) * 0.50f * feature;
		density += noise.Noise(n * 1.01f) * 1.00f * feature;
		v.w = density;
		return v;
	}

};

struct DensityPoint
{
	Vector3f point;
	float density;
	float biome;
	Vector3f unused;
};

MarchingCubes::MarchingCubes(RenderDevice* renderDevice,
	Shader* shaderMarchingCubes, Shader* shaderNoise,
	const MaterialComponent& material,
	ECS& ecs) :
	m_renderDevice(renderDevice),
	m_shaderMarchingCubes(shaderMarchingCubes),
	m_terrainMaterial(material),
	m_shaderNoise(shaderNoise),
	m_ecs(ecs),
	m_chunkResolution(32),
	m_chunkSize(120.0f),
	m_focus(Vector3f::ZERO)
{
	m_maxLODCount = 5;

	m_radius = m_chunkSize.x * 4;
	m_lodRadius = m_chunkSize.x * 1.0f;
	AddComponentType<TransformComponent>();
	AddComponentType<MeshComponent>();
	AddComponentType<MaterialComponent>();
	AddComponentType<Chunk>();

	// Create buffers
	uint32 zero = 0;
	uint32 MAX_VERTICES = (m_chunkResolution.x + 3) * (m_chunkResolution.y + 3);
	uint32 pointCount = (1 + m_chunkResolution.x) * (1 + m_chunkResolution.y) * (1 + m_chunkResolution.z);
	m_bufferAtomicCounter.BufferData(1, &zero); // GL_DYNAMIC_DRAW
	m_bufferLookup.BufferData<int32>(
		MARCHING_CUBES_LOOKUP_TABLE_SIZE,
		MARCHING_CUBES_LOOKUP_TABLE); // GL_STATIC_DRAW
	m_bufferPoints.BufferData<DensityPoint>(pointCount, nullptr);  // GL_DYNAMIC_DRAW
	m_bufferVertices.BufferData<VertexPosTexNorm>(MAX_VERTICES, nullptr);

	// Create index buffers for different LOD resolutions
	Vector2ui resolution = m_chunkResolution.xy;
	for (uint32 lod = 0; lod < m_maxLODCount; lod++)
	{
		m_bufferIndices[lod].BufferData<uint32>(resolution.x * resolution.y * 6, nullptr);
		uint32* indices = m_bufferIndices[lod].MapBufferDataWrite<uint32>();
		uint32 indexIndex = 0;

		for (uint32 y = 0; y < resolution.y; y++)
		{
			for (uint32 x = 0; x < resolution.x; x++)
			{
				uint32 i0 = ((y + 1) * (resolution.x + 3)) + (x + 1);
				uint32 i1 = i0 + 1;
				uint32 i2 = i0 + (resolution.x + 3);
				uint32 i3 = i2 + 1;
				indices[indexIndex++] = i0;
				indices[indexIndex++] = i1;
				indices[indexIndex++] = i3;
				indices[indexIndex++] = i0;
				indices[indexIndex++] = i3;
				indices[indexIndex++] = i2;
			}
		}
		m_bufferIndices[lod].UnmapBufferData();
		resolution /= 2;
	}
}

MarchingCubes::~MarchingCubes()
{
}

void MarchingCubes::SetFocus(const Vector3f& focus)
{
	m_focus = focus;
}

void MarchingCubes::SetMarchingCubesShader(Shader * shader)
{
	m_shaderMarchingCubes = shader;
}

void MarchingCubes::SetDensityShader(Shader * shader)
{
	m_shaderNoise = shader;
}

void MarchingCubes::SetHeightmapShader(Shader * shader)
{
	m_shaderHeightmap = shader;
}

void MarchingCubes::PreUpdate(float timeDelta)
{
	Vector3i center;
	Vector3i mins;
	Vector3i maxs;
	Vector3i coord;
	for (uint32 i = 0; i < 3; i++)
	{
		mins[i] = (int) Math::Floor((m_focus[i] - m_radius) / m_chunkSize[i]);
		maxs[i] = (int) Math::Ceil((m_focus[i] + m_radius) / m_chunkSize[i]);
	}
	mins.z = 0;
	maxs.z = 1;

	// Remove far chunks
	Array<Vector3i> updatedCoords;
	Array<Vector3i> coords;
	for (auto it = m_chunkMap.begin(); it != m_chunkMap.end(); it++)
		coords.push_back(it->first);
	for (uint32 i = 0; i < coords.size(); i++)
	{
		coord = coords[i];
		float dist = CalcDistToChunk(coord);
		uint32 lodIndex = CalcLODIndex(coord);
		Chunk* chunk = m_ecs.GetComponent<Chunk>(m_chunkMap[coord]);
		if (dist >= m_radius)
		{
			RemoveChunk(coord);
			updatedCoords.push_back(coord);
		}
		else if (chunk->GetLODIndex() != lodIndex)
		{
			CreateChunk(coord);
			updatedCoords.push_back(coord);
		}
	}
	
	// Create near chunks
	coord = Vector3i::ZERO;
	for (coord.z = mins.z; coord.z < maxs.z; coord.z++)
	{
		for (coord.y = mins.y; coord.y < maxs.y; coord.y++)
		{
			for (coord.x = mins.x; coord.x < maxs.x; coord.x++)
			{
				float dist = CalcDistToChunk(coord);
				if (m_chunkMap.find(coord) == m_chunkMap.end() && dist < m_radius)
				{
					CreateChunk(coord);
					updatedCoords.push_back(coord);
				}
			}
		}
	}

	// Sew seams
	for (auto it = m_chunkMap.begin(); it != m_chunkMap.end(); it++)
	{
		coord = it->first;
		/*if (updatedCoords.find(coord) != updatedCoords.end() ||
			updatedCoords.find(coord + Vector3i(1, 0, 0)) != updatedCoords.end() ||
			updatedCoords.find(coord + Vector3i(-1, 0, 0)) != updatedCoords.end() ||
			updatedCoords.find(coord + Vector3i(0, 1, 0)) != updatedCoords.end() ||
			updatedCoords.find(coord + Vector3i(0, -1, 0)) != updatedCoords.end())*/
		{
			SewSeam(coord, coord + Vector3i(1, 0, 0));
			SewSeam(coord, coord + Vector3i(-1, 0, 0));
			SewSeam(coord, coord + Vector3i(0, 1, 0));
			SewSeam(coord, coord + Vector3i(0, -1, 0));
		}
	}
}

void MarchingCubes::UpdateComponents(float delta, BaseECSComponent** components)
{
	TransformComponent* transform = (TransformComponent*) components[0];
	MeshComponent* meshComponent = (MeshComponent*) components[1];
	MaterialComponent* material = (MaterialComponent*) components[2];
	Chunk* chunk = (Chunk*) components[3];


}

void MarchingCubes::RecreateChunks()
{
	Array<Vector3i> coords;
	for (auto it = m_chunkMap.begin(); it != m_chunkMap.end(); it++)
		coords.push_back(it->first);
	for (uint32 i = 0; i < coords.size(); i++)
		CreateChunk(coords[i]);
}

uint32 MarchingCubes::CalcLODIndex(const Vector3i & coord)
{
	float dist = CalcDistToChunk(coord);
	uint32 lodIndex = (uint32) (dist / m_lodRadius);
	while (m_chunkResolution.x / (1 << lodIndex) < 2)
		lodIndex--;
	return lodIndex;
}

float MarchingCubes::CalcDistToChunk(const Vector3i & coord)
{
	Bounds bounds;
	bounds.mins.xy = Vector2f(coord.xy) * m_chunkSize.xy;
	bounds.mins.z = m_focus.z;
	bounds.maxs = bounds.mins;
	bounds.maxs.xy += m_chunkSize.xy;
	return bounds.DistToPoint(m_focus);
}

void MarchingCubes::RemoveChunk(const Vector3i& coord)
{
	m_ecs.RemoveEntity(m_chunkMap[coord]);
	m_chunkMap.erase(coord);
}

Chunk* MarchingCubes::GetChunk(const Vector3i& coord)
{
	if (m_chunkMap.find(coord) != m_chunkMap.end())
		return m_ecs.GetComponent<Chunk>(m_chunkMap[coord]);
	return nullptr;
}

void MarchingCubes::SewSeam(const Vector3i& coord, const Vector3i& neighborCoord)
{
	Chunk* chunk = GetChunk(coord);
	Chunk* neighbor = GetChunk(neighborCoord);
	if (chunk == nullptr || neighbor == nullptr)
		return;
	MeshComponent* chunkMesh = m_ecs.GetComponent<MeshComponent>(m_chunkMap[coord]);
	MeshComponent* neighborMesh = m_ecs.GetComponent<MeshComponent>(m_chunkMap[neighborCoord]);

	uint32 lodIndex = chunk->GetLODIndex();
	int32 lodDiff = (int) neighbor->GetLODIndex() - (int) chunk->GetLODIndex();
	if (lodDiff <= 0)
		return;

	uint32 step = 1u << (uint32) lodDiff;
	Vector3ui resolution = m_chunkResolution / (1u << chunk->GetLODIndex());
	VertexPosTexNorm* vertices = chunkMesh->mesh->GetVertexData()->GetVertexBuffer()->MapBufferDataWrite<VertexPosTexNorm>();
	neighborMesh->mesh->GetVertexData()->GetVertexBuffer()->MapBufferDataWrite<VertexPosTexNorm>();
	Vector2i start = Vector2i(0);
	Vector2i dir = Vector2i(1, 0);
	if (neighborCoord.x > coord.x)
	{
		start = Vector2i(resolution.x, 0);
		dir = Vector2i(0, 1);
	}
	else if (neighborCoord.x < coord.x)
	{
		start = Vector2i(0, 0);
		dir = Vector2i(0, 1);
	}
	else if (neighborCoord.y > coord.y)
	{
		start = Vector2i(0, resolution.y);
		dir = Vector2i(1, 0);
	}
	else
	{
		start = Vector2i(0, 0);
		dir = Vector2i(1, 0);
	}
	for (uint32 i = 0; i < resolution.x; i += step)
	{
		Vector3f first = vertices[GetVertexIndex(start + (dir * i), lodIndex)].position;
		Vector3f last = vertices[GetVertexIndex(start + (dir * (i + step)), lodIndex)].position;
		for (uint32 j = 1; j < step; j++)
		{
			float t = (float) j / (float) step;
			Vector2i point = start + (dir * (i + j));
			vertices[GetVertexIndex(point, lodIndex)].position.z = Math::Lerp(first.z, last.z, t);
		}
	}
	chunkMesh->mesh->GetVertexData()->GetVertexBuffer()->UnmapBufferData();
	neighborMesh->mesh->GetVertexData()->GetVertexBuffer()->UnmapBufferData();
}

uint32 MarchingCubes::GetVertexIndex(Vector2i point, uint32 lodIndex) const
{
	Vector3ui resolution = m_chunkResolution / (1 << lodIndex);
	return (uint32) (((point.y + 1) * (resolution.x + 3)) + (point.x + 1));
}

EntityHandle MarchingCubes::CreateChunk(const Vector3i& coord)
{
	if (m_chunkMap.find(coord) != m_chunkMap.end())
	{
		RemoveChunk(coord);
	}

	uint32 lodIndex = CalcLODIndex(coord);

	//printf("Creating chunk for coord %d, %d, %d,  lod=%u\n", coord.x, coord.y, coord.z, lodIndex);
	Vector3f offset = Vector3f(m_chunkSize) * Vector3f(coord);
	Mesh* mesh = nullptr;
	if (coord.z == 0)
		mesh = CreateMesh(offset, lodIndex);
	TransformComponent transform;
	Chunk chunk(coord, lodIndex);
	EntityHandle entity;

	if (mesh != nullptr)
	{
		MeshComponent meshComponent;
		meshComponent.mesh = mesh;
		entity = m_ecs.CreateEntity(
			transform, meshComponent, m_terrainMaterial, chunk);
	}
	else
	{
		entity = m_ecs.CreateEntity(transform, chunk);
	}

	m_chunkMap[coord] = entity;
	return entity;
}

Mesh* MarchingCubes::CreateMesh(const Vector3f& offset, uint32 lodIndex)
{
	uint32 WORK_GROUP_SIZE = 8;
	uint32 TERRAIN_WORK_GROUP_SIZE = 8;
	Vector3ui resolution = m_chunkResolution / (1 << lodIndex);
	uint32 vertexCount = (resolution.x + 3) * (resolution.y + 3);

	// Reset atomic counter to 0
	uint32 zero = 0;
	m_bufferAtomicCounter.BufferSubData(0, 1, &zero);

	// Generate heightmap vertices
	WORK_GROUP_SIZE = 4;
	m_renderDevice->SetShaderUniform(m_shaderGenerateVertices, "u_size", m_chunkSize).Ignore();
	m_renderDevice->SetShaderUniform(m_shaderGenerateVertices, "u_resolution", resolution).Ignore();
	m_renderDevice->SetShaderUniform(m_shaderGenerateVertices, "u_offset", offset).Ignore();
	m_renderDevice->SetShaderUniform(m_shaderGenerateVertices, "u_floorPosition", m_chunkSize.z * 0.3f).Ignore();
	m_renderDevice->BindBuffer(m_bufferVertices, 0);
	m_renderDevice->DispatchCompute(m_shaderGenerateVertices,
		Math::Max(1u, ((resolution.x + 3) / WORK_GROUP_SIZE) + 1),
		Math::Max(1u, ((resolution.y + 3) / WORK_GROUP_SIZE) + 1),
		1);

	// Calculate normals
	WORK_GROUP_SIZE = 4;
	m_renderDevice->SetShaderUniform(m_shaderGenerateNormals, "u_resolution", resolution).Ignore();
	m_renderDevice->BindBuffer(m_bufferVertices, 0);
	m_renderDevice->DispatchCompute(m_shaderGenerateNormals,
		Math::Max(1u, ((resolution.x + 1) / WORK_GROUP_SIZE) + 1),
		Math::Max(1u, ((resolution.y + 1) / WORK_GROUP_SIZE) + 1),
		1);

	/*
	// Reset atomic counter to 0
	uint32 zero = 0;
	m_bufferAtomicCounter.BufferSubData(0, 1, &zero);

	// Run the terrain generation shader
	//printf("Generating noise\n");
	m_renderDevice->SetShaderUniform(m_shaderNoise, "u_size", m_chunkSize);
	m_renderDevice->SetShaderUniform(m_shaderNoise, "u_resolution", resolution);
	m_renderDevice->SetShaderUniform(m_shaderNoise, "u_offset", offset);
	m_renderDevice->SetShaderUniform(m_shaderNoise, "u_floorPosition", m_chunkSize.z * 0.3f);
	m_renderDevice->BindBuffer(m_bufferPoints, 0);
	m_renderDevice->DispatchCompute(m_shaderNoise,
		(resolution.x / TERRAIN_WORK_GROUP_SIZE) + 1,
		(resolution.y / TERRAIN_WORK_GROUP_SIZE) + 1,
		(resolution.z / TERRAIN_WORK_GROUP_SIZE) + 1);

	// Run the compute shader
	//printf("Running Marching Cubes\n");
	m_renderDevice->SetShaderUniform(m_shaderMarchingCubes, "u_resolution", resolution);
	m_renderDevice->BindBuffer(m_bufferVertices, 0);
	m_renderDevice->BindBuffer(m_bufferPoints, 1);
	m_renderDevice->BindBuffer(m_bufferLookup, 2);
	m_renderDevice->BindBuffer(m_bufferAtomicCounter, 3);
	m_renderDevice->BindBuffer(m_bufferIndices, 4);
	m_renderDevice->DispatchCompute(m_shaderMarchingCubes, resolution / WORK_GROUP_SIZE);
	*/
	// Read the atomic counter (the number of triangles)
	/*
	const uint32* counter = m_bufferAtomicCounter.MapBufferDataRead<uint32>();
	uint32 vertexCount = *counter * 3;
	m_bufferAtomicCounter.UnmapBufferData();
	
	// Copy vertices and indices into the mesh buffers
	//printf("vertexCount = %u\n", vertexCount);
	if (vertexCount == 0)
		return nullptr;
	CMG_ASSERT(vertexCount <= m_bufferVertices.GetSize());
	*/


	Mesh* mesh = new Mesh();
	mesh->GetVertexData()->BufferVertices(vertexCount, (const VertexPosTexNorm*) nullptr);
	mesh->GetVertexData()->GetVertexBuffer()->BufferData(
		0, vertexCount * sizeof(VertexPosTexNorm), m_bufferVertices);
	/*mesh->GetIndexData()->GetIndexBuffer()->BufferData(
		0, vertexCount * sizeof(uint32), m_bufferIndices);*/
	mesh->GetIndexData()->GetIndexBuffer()->BufferData(m_bufferIndices[lodIndex]);
	mesh->GetIndexData()->SetIndexRange(0,
		m_bufferIndices[lodIndex].GetSize() / sizeof(uint32));

	//printf("Done!\n");
	return mesh;
}

