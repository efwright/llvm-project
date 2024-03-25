#include <omp.h>
#include <stdio.h>

void foo()
{
  #pragma omp target parallel
  {
    printf("Hello world\n");
  }
}

int main()
{
  foo();
}

