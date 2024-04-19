#include <omp.h>
#include <stdio.h>

void foo()
{
  printf("foo! %i\n", omp_get_thread_num());
}

int main()
{
#pragma omp target teams num_teams(1)
{

#pragma omp parallel num_threads(1)
{

  #pragma omp simd
  for(int i = -10; i > -12; i--)
    foo();

}

}
}

