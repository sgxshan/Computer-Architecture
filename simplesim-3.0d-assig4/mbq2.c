#include<stdio.h>
#include <stdlib.h>

#define size 6400000

int main(int argc, char *argv[]){
	int a[size];
	int sum = 0;
	int i;
	/*For array a, if we fetch i = i+50 in every cycle, the stride state is always steady 
	then every data we need would be prefetched in previous cycle, 
	so the data miss rate will approximately be 0. our simulation result is 0.06%.*/
	
	/*
	for(i = 0; i<size; i+=50){
		sum += a[i];
	}
	*/

	/*
	In the following code,we fetch the element 51st,100th,151st,200th... in every cycle, so the prefetcther state will always be no_prediction
	thus the prefetched data will always be useless
	the data miss rate will be 100%. our simulation result is 96.66%.*/

	int j=1;
	for(i = 0; i+j<size; i+=50){
		sum += a[i+j];
		j = j==1? 0:1;
	}

	/*Therefore, our nextline prefetcher is proved to be correct.*/

	printf("%d",a, sum);
	return 0; 
} 
