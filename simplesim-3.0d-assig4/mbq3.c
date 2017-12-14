#include<stdio.h>
#include <stdlib.h>

#define size 6400000

int main(int argc, char *argv[]){
	int a[size];
	int sum = 0;
	int i;
	int j;

	/*For array a, there are totally 40 missing data patterns and is all stored in the global history buffer
	so the data miss rate will approximately be 0. our simulation result is 0.2%.*/
	
	/*
	for(i = 0; i<size; i+=50){
		j = i%2000;
		sum += a[j];
	}
	*/
	

	/*
	In the following code,if we fetch i = i+50 in every cycle, every missing data pattern only appear once
	thus the prefetched data will always be useless
	the data miss rate will be 100%. our simulation result is 96.81%.*/
	
	
	for(i = 0; i<size; i+=50){
		sum += a[i];
	}
	
	

	/*Therefore, our nextline prefetcher is proved to be correct.*/

	printf("%d",a, sum);
	return 0; 
} 
