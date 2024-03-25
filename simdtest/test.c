#include <omp.h>
#include <stdio.h>

void foo()
{
  #pragma omp target parallel num_threads(2)
  {
    #pragma omp simd
    for(int i = 0; i < 10; i++)
    {
      printf("(%i, %i)\n", omp_get_thread_num(), i);
    }
  }
}

int main()
{
  foo();
}

