/*
 * readcsv.h
 *
 *  Created on: 16 Aug 2024
 *      Author: @actionphysics
 */

#ifndef READCSV_H_
#define READCSV_H_

typedef union cell_value_u
{
	long i;
	double d;
	char* s;
} cell_value;

typedef enum {NOWT, LONG, DOUBLE, DATETIME, DATE, TIME, STRING} cell_type;

typedef struct cell_s
{
	cell_value v;
	cell_type t;
} cell;

int readcsv(char* fname, int* rows, int* columns, cell*** result);


#endif /* READCSV_H_ */
