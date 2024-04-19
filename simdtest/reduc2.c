#include <stdio.h>
#include <omp.h>

int main()
{
  #pragma omp target teams
  {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for(int i = 0; i < 128; i++) {
      sum++;
    }
  }

  return 0;
}

