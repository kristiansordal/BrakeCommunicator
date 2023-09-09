#include <stdio.h>
#include <time.h>

#define size 1000000

int main() {
    int i, j;
    int data[size];
    clock_t start, end;
    int sum;

    sum = 0;

    start = clock();
    for (j = 0; j < 100; j++)
        for (i = 0; i < size; i++)
            sum = sum + data[i];
    end = clock();

    printf("Time taken %f \n", (end - start) / (float)CLOCKS_PER_SEC);
    printf("The sum is %d \n", sum);
    printf("Time per element %10.9f \n", (end - start) / (float)CLOCKS_PER_SEC / (100 * size));
}
