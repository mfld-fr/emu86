#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <assert.h>

//#include "op-id.h"
//#include "op-id-name.h"
#include "op-common.h"
//#include "op-class.h"


#define MAXSYMBOLS	1000
struct map { int addr; char *name; };
struct map map[MAXSYMBOLS];
int numsymbols = 0;

static int qscomp (const void *s1, const void *s2)
	{
	struct map *m1 = (struct map *)s1;
	struct map *m2 = (struct map *)s2;

	return (m1->addr - m2->addr);
	}

int map_file_load (char *filename)
	{
	FILE *fp;
	char buf[128];

	fp = fopen(filename, "r");
	if (!fp)
		{
		printf("Can't open map file: %s\n", filename);
		return 0;
		}

	while(fgets(buf, sizeof(buf), fp) != NULL)
		{
		int addr;
		unsigned char type;
		char name[32];

		if (strchr(buf, '!') || strchr(buf, '&'))
			continue;
		if (sscanf(buf, "%x %c %s", &addr, &type, name) == 3)
			{
			if (numsymbols >= MAXSYMBOLS)
				{
				printf("info: Too many symbols: %s\n", filename);
				break;
				}
			// 0x10000+ is text
			// 0x20000+ is far text
			// 0x30000+ is data
			if (addr >= 0x10000 && addr < 0x20000)
				{
				map[numsymbols].addr = addr;
				map[numsymbols].name = strdup(name);
				numsymbols++;
				}
			}
		}
	qsort(map, numsymbols, sizeof(struct map), qscomp);

	fclose(fp);
	return 1;
	}

// map near text address to symbol
// TODO: far_text_symbol and data_name
char *text_symbol (int addr)
	{
	int i = 1;
	static char buf[32];

	addr += 0x10000;	// adjust to mapfile text symbol
	while (addr >= map[i].addr)
		{
		if (i >= MAXSYMBOLS)
			{
			sprintf(buf, "%.4x", addr);	// fail, display hex address
			return buf;
			}
		i++;
		}
	if (addr - map[i-1].addr)
		sprintf(buf, "%s+%x", map[i-1].name, addr - map[i-1].addr);
	else sprintf(buf, "%s", map[i-1].name);
	return buf;
	}
