/*
 * readcsv.c
 *
 *  Created on: 16 Aug 2024
 *      Author: @actionphysics
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <regex.h>
#include <time.h>

#include <inc/readcsv.h>

#define MAXLIN 1024

regex_t int_pattern;
regex_t double_pattern;
regex_t datetime_pattern;
regex_t date_pattern;
regex_t time_pattern;

int
substitute_in_strings(char* line, unsigned char old, unsigned char new)
{
	enum {QUOTED, UNQUOTED} inQuotes = UNQUOTED;	// initial token usually not quoted
	char* pt = line;
	if (line[0] == '"')
		inQuotes = QUOTED;	// less usual case
	while (*pt != '\0')
	{
		if (*pt == '"')
		{	// toggle quotation state on finding each "
			if (inQuotes == UNQUOTED)
				inQuotes = QUOTED;
			else
				inQuotes = UNQUOTED;
		}
		if (inQuotes == QUOTED && *pt == old)
			*pt = new;
		pt++;
	}
	//fprintf(stderr,"%s", line);
	return 0;
}

int
tokenize(char* line, char* delim, char*** tokens)
{
	// find how many tokens
	int num_tokens = 1;
	char* pt;
	for (pt=line; *pt!='\0'; pt++)
		if (*pt == delim[0])
			num_tokens++;
	// use number of tokens to size the return array
	*tokens = malloc(num_tokens * sizeof(char*));
	// copy tokens
	num_tokens = 0;
	char* start_pt = line;
	char* end_pt = line;
	while (*start_pt)	// start ptr hasn't reached end of line
	{
		while (*end_pt && *end_pt != '\n' && *end_pt != delim[0])
		{
			end_pt++;
		}
		if (end_pt == start_pt)
		{	// null token (for example ^, or ,, in CSV)
			(*tokens)[num_tokens] = strdup("");
		}
		else
		{
			(*tokens)[num_tokens] = strndup(start_pt, end_pt-start_pt);
		}
		num_tokens++;
		start_pt = end_pt + 1;
		end_pt = start_pt;
	}
	return num_tokens;
}

int
find_dimensions(char* filename, int* rows, int* columns)
{
	char line[MAXLIN];
	char* pt;
	char** tokens;
	int num_tokens;
	int first_columns = -1;
	int row_count = 0;
	FILE* csv_file = fopen(filename,"r");
	while ((pt = fgets(line, MAXLIN, csv_file)))
	{
		if (strlen(line) < 2)
			continue;
		substitute_in_strings(line, ',', '`');
		num_tokens = tokenize(line, ",", &tokens);
		if (first_columns == -1)
		{
			first_columns = num_tokens;
		}
		else
		{
			if (num_tokens != first_columns)
				return -1;
		}
		row_count++;
//		for (int i=0; i<num_tokens; i++)
//			fprintf(stderr, "%d: %s ", i, tokens[i]);
//		fprintf(stderr,"\n");
	}
	fclose(csv_file);
	*columns = first_columns;
	*rows = row_count;
	return 0;
}

int
create_data_structure(int r, int c, cell*** array)
{
	*array = malloc(r*c*sizeof(cell) + r*sizeof(cell*));
	if ((*array == NULL))
		return -1;
	for (int i=0; i<r; i++)
	{
		(*array)[i] = (void*)(*array) + r*sizeof(cell*) + i*c*sizeof(cell);
	}
	return 0;
}

int
read_file_to_strings(char* filename, int r, int c, cell** array)
{
	char line[MAXLIN];
	char* pt;
	char** tokens;
	int row_count = 0;
	FILE* csv_file = fopen(filename,"r");
	while ((pt = fgets(line, MAXLIN, csv_file)))
	{
		substitute_in_strings(line, ',', '`');
		tokenize(line, ",", &tokens);
		for (int i=0; i<c; i++)
		{
			array[row_count][i].v.s = strdup(tokens[i]);
			array[row_count][i].t = STRING;
		}
		row_count++;
	}
	fclose(csv_file);
	return 0;
}

int
parseDateTime(char* s, int* d, int* m, int* y, int* hh, int* mm, int* ss)
{	// assume dd/mm/yy[yy] hh:mm:ss format
	char* sub = strtok(s, "/");
	if (sub)
		if (1 != sscanf(sub, "%d", d))
			return -1;
	sub = strtok(NULL, "/");
	if (sub)
		if (1 != sscanf(sub, "%d", m))
			return -1;
	sub = strtok(NULL, "/ ");
	if (sub)
		if (1 != sscanf(sub, "%d", y))
			return -1;
	sub = strtok(NULL, ":");
	if (sub)
		if (1 != sscanf(sub, "%d", hh))
			return -1;
	sub = strtok(NULL, ":");
	if (sub)
		if (1 != sscanf(sub, "%d", mm))
			return -1;
	sub = strtok(NULL, ":");
	if (sub)
		if (1 != sscanf(sub, "%d", ss))
			return -1;
	return 0;
}

int
parseDate(char* s, int* d, int* m, int* y)
{	// assume dd/mm/yy format
	char* sub = strtok(s, "/");
	if (sub)
		if (1 != sscanf(sub, "%d", d))
			return -1;
	sub = strtok(NULL, "/");
	if (sub)
		if (1 != sscanf(sub, "%d", m))
			return -1;
	sub = strtok(NULL, "/");
	if (sub)
		if (1 != sscanf(sub, "%d", y))
			return -1;
	return 0;
}

int
parseTime(char* s, int* hh, int* mm, int* ss)
{	// assume dd/mm/yy[yy] hh:mm:ss format
	char* sub = strtok(s, ":");
	if (sub)
		if (1 != sscanf(sub, "%d", hh))
			return -1;
	sub = strtok(NULL, ":");
	if (sub)
		if (1 != sscanf(sub, "%d", mm))
			return -1;
	sub = strtok(NULL, ":");
	if (sub)
		if (1 != sscanf(sub, "%d", ss))
			return -1;
	return 0;
}

int
string_to_value(cell* cpt)
{
	long long_int_value;
	double double_value;
	struct tm tm;
	//double date_value;
	if (cpt->t != STRING)
		return -1;
	char* s = strdup(cpt->v.s);
	if (0 == strlen(s))
	{
		cpt->v.i = 0L;
		cpt->t = NOWT;
		return 0;
	}
	if (0 == regexec(&datetime_pattern, s, 0, NULL, 0))
	{
		int d, m, y, hh, mm, ss;
		cpt->t = DATETIME;
		memset(&tm, 0, sizeof(struct tm));
		parseDateTime(s, &d, &m, &y, &hh, &mm, &ss);
		tm.tm_mday = d;
		tm.tm_mon = m - 1;	// struct tm months are 0 - 11
		if (y < 38)	// fix 2-digit year to human 4-digit year
			y += 2000;
		else if (y < 100)
			y += 1900;
		tm.tm_year = y - 1900;	// struct tm convention for year
		tm.tm_hour = hh;
		tm.tm_min = mm;
		tm.tm_sec = ss;
		cpt->v.i = (long)mktime(&tm);
		return 0;
	}
	if (0 == regexec(&date_pattern, s, 0, NULL, 0))
	{
		int d, m, y;
		cpt->t = DATE;
		memset(&tm, 0, sizeof(struct tm));
		parseDate(s, &d, &m, &y);
		tm.tm_mday = d;
		tm.tm_mon = m - 1;	// struct tm months are 0 - 11
		if (y < 38)	// fix 2-digit year to human 4-digit year
			y += 2000;
		else if (y < 100)
			y += 1900;
		tm.tm_year = y - 1900;	// struct tm convention for year
		cpt->v.i = (long)mktime(&tm);
		return 0;
	}
	if (0 == regexec(&time_pattern, s, 0, NULL, 0))
	{
		int hh, mm, ss;
		cpt->t = TIME;
		memset(&tm, 0, sizeof(struct tm));
		parseTime(s, &hh, &mm, &ss);
		tm.tm_hour = hh;
		tm.tm_min = mm;
		tm.tm_sec = ss;
		tm.tm_year = 70; // struct tm convention for time origin
		tm.tm_mday = 1;
		cpt->v.i = (long)mktime(&tm);
		return 0;
	}
	if (0 == regexec(&double_pattern, s, 0, NULL, 0))
	{
		sscanf(s, "%lg", &double_value);
		cpt->v.d = double_value;
		cpt->t = DOUBLE;
		return 0;
	}
	if (0 == regexec(&int_pattern, s, 0, NULL, 0))
	{
		sscanf(s, "%ld", &long_int_value);
		cpt->v.i = long_int_value;
		cpt->t = LONG;
		return 0;
	}
	return 0;
}

void
strings_to_values(int r, int c, cell** array)
{
	for (int i=0; i<r; i++)
		for (int j=0; j<c; j++)
			string_to_value(array[i]+j);
}

int
readcsv(char* fname, int* rows, int* columns, cell*** result)
{
	regcomp(&int_pattern, "[-+]?[0-9]+", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
	regcomp(&double_pattern, "[-+]?[0-9]+\\.[0-9]+", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
	regcomp(&datetime_pattern, "[0-9]{1,2}/[0-9]{1,2}/[0-9]{2,4} [0-9]{1,2}:[0-9]{1,2}:[0-9]{2,4}", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
	regcomp(&date_pattern, "[0-9]{1,2}/[0-9]{1,2}/[0-9]{2,4}", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
	regcomp(&time_pattern, "[0-9]{1,2}:[0-9]{1,2}(:[0-9]{1,2})?", REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
	if (-1 == find_dimensions(fname, rows, columns))
		return -1;
	if (-1 == create_data_structure(*rows, *columns, result))
		return -1;
	if (-1 == read_file_to_strings(fname, *rows, *columns, *result))
		return -1;
	strings_to_values(*rows, *columns, *result);
	return 0;
}

