#include <stdio.h>
#include <stdlib.h>

int main(int ac, char **av)
{
	FILE *fp;
	char buf[128];

	fp = fopen("file", "r");
	if (!fp) exit(1);

	printf("struct map { int addr; char *name; };\n");
	printf("struct map map[] = {\n");
	while(fgets(buf, sizeof(buf), fp) != NULL)
	{
		int addr;
		unsigned char type;
		char name[32];

		if (sscanf(buf, "%x %c %s", &addr, &type, name) == 3)
		{
			// 0x10000+ is text
			// 0x20000+ is far text
			// 0x30000+ is data
			if (addr >= 0x10000 && addr < 0x20000)
			{
				// TODO keep 0x10000 in address for near text
				// otherwise seperate text, fartext and data maps required
				addr -= 0x10000;
				printf("{ 0x%04x, \"%s\" },\n", addr, name);
			}
		}
	}
	printf("{ -1, 0} };\n");
	fclose(fp);
}
