#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void test1() 
{
	const int NUM_HEIGHTS = 3;
	int *heights = malloc( NUM_HEIGHTS );
	for (int i=0; i < NUM_HEIGHTS; i++) {
		heights[i] = i * i;
		printf("%d: %d\n", i, heights[i]);
	}
}

void test2() 
{
	const int NUM_WEIGHTS = 5;
	int *weights = malloc( NUM_WEIGHTS * sizeof(int));
	for (int i=0; i < NUM_WEIGHTS; i++) {
		weights[i] = 100 + i;
		printf("%d: %d\n", i, weights[i]);
	}
	free (weights);
	weights[0] = 0;
}

char *get_string()
{
	char message[100] = "Hello world!";
	char *ret = message;
	return ret;
}

void test3()
{
	printf( "String: %s\n", get_string() );
}

int main(int agra, char* args[]) 
{
	test1();
	test2();
	test3();
	
	return 0;
}

