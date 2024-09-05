# readcsv

This is a tiny C Makefile project to read a comma-separated-value (CSV) text file representing an array of data. The project generates a shared library, libreadcsv.so

The CSV file it reads is assumed to have one line per row of the array. Every line must contain the same number of comma-separated fields representing the column entries for that row.

inc/readcsv.h typedefs an enum cell_type and a union cell_value, then typedefs a structure cell consisting of a cell_type and a cell_value.

The cell_value union has three options, a long integer, a double, and a char* string pointer

The supported cell_types are:  
NOWT      - an empty cell ("nowt" is a Yorkshire dialect word meaning "nothing", because NULL is reserved)  
LONG      - long integer to hold any integer value of a cell matching   
DOUBLE    - double to hold any numerical value with a decimal point  
DATETIME  - the cell value was a date and time such as "21/6/2024 07:30:59". The value is stored in the long integer field as a "Unix epoch time"  
DATE      - the cell value was a date such as "21/6/2024". The value (midnight on that date) is stored in the long integer field as a "Unix epoch time"  
TIME      - the cell value was a time such as "07:30:59". The value is stored in the long integer field in seconds since midnight  
STRING    - any characters found in the cell which do not match any of the preceding formats are stored as a C string using the char* pointer to a string from strdup()  

The only function defined in readcsv.h is

int readcsv(char* fname, int* rows, int* columns, cell*** result)

which returns 0 for success and -1 for any failure.

Typical usage is:

char* filename = "myfile.csv";  
int r, c;  
cell** data;  
if (-1 == readcsv(filename, &r, &c, &data)  
  exit(EXIT_FAILURE);  
// then data[i][j] is a "cell", so use code such as...  
for (int i=0; i<r; i++)  
{  
  for (int j=0; j<c; j++)  
  {  
    if (data[i][j].t == LONG)  
      printf("%ld ", data[i][j].v.i);  
  }  
  printf("\n");  
}  

This project is maintained by @actionphysics
