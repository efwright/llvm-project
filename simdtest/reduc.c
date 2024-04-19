#include <stdio.h>
#include <omp.h>

int main()
{
  #pragma omp target parallel num_threads(32)
  {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for(int i = 0; i < 32; i++) {
      sum++;
    }
  }

  return 0;
}

