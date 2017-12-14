#include <stdio.h>

int  main (int argc, char *argv[])
{
    int i;
    int sum = 0;
  
   // if ( argc != 2 ){
   //    printf("Usage: %s <count>\n", argv[0]);
   //     exit(5);
   // }

   for (i = atoi(argv[1]); i > 0; i--){
       sum += i;
       sum += 1; 
   }

    printf("Sum = %d\n", sum);

    return 0;
}
