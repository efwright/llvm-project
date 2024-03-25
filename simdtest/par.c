#include <omp.h>
#include <stdio.h>

void foo()
{
  #pragma omp target parallel
  {
    printf("Hello OMP!\n");
  }
}

int main()
{
  foo();
}

