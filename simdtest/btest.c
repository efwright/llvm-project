#include <stdio.h>
#include <omp.h>

int main()
{
  #pragma omp target teams num_teams(1)
  {
    printf("Hello OpenMP!\n");
  }
}

