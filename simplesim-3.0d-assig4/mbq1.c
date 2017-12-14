#include<stdio.h>
#define size 6400000

int main(int argc, char *argv[]){
	int a[size];
	int sum = 0;
	int i;
	/*For array a, if we fetch i = i+16 in every cycle(every integer is 4 bytes), 
	then every data we need would be prefetched in previous cycle, 
	so the data miss rate will approximately be 0. our simulation result is 0.01%.*/
	
	/*
	for(i = 0; i<size; i+=50){
		sum += a[i];
	}*/

	/*
	if we fetch i = i+50 in every cycle, the prefetched data will always be useless
	the data miss rate will be 100%. our simulation result is 96.66%.

	*/	
	for(i = 0; i<size; i+=50){
		sum += a[i];
	}

	/*Therefore, our nextline prefetcher is proved to be correct.*/

	printf("%d",a, sum);
	return 0; 
} 
