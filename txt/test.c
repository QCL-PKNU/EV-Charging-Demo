#include <stdio.h>
#include <stdlib.h>

int slide[10] = {0, };

int getmax() 
{
	int i;

	int max = slide[0];
	
	for(i = 1 ; i < 10; i++)
	{
		if(max < slide[i]) 
		{
			max = slide[i];
		}
	}

	return max;
}

int main()
{
	FILE *fd = fopen("temp1.txt", "r");
	FILE *wd = fopen("temp1_out.txt", "w");

	char readBuf[16];
	int readValue;
	int i = 0;

	while(fscanf(fd, "%s\n", readBuf) != EOF)
	{
		readValue = atoi(readBuf);
		readValue = (readValue - 37200);	
		if(readValue < 0) readValue = 0;
		slide[i] = readValue;
		
		//fprintf(wd, "%d - %d - %d\n", i, readValue, getmax());	
		fprintf(wd, "%d\n", getmax());	
		i = (i + 1) % 10;
	}

	fclose(fd);
	fclose(wd);
}
