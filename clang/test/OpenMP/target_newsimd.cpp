// RUN: %clang_cc1 -verify -fopenmp -fopenmp-targets=nvptx64 %s -Wuninitialized

void simd1()
{
  #pragma omp target parallel
  {
    #pragma omp simd
    for(int i = 0; i < 10; i++)
    {

    } 
  }
}


