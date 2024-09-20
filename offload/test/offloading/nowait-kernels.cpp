#include <stdio.h>
#include <omp.h>

#define SUCCESS 0
#define FAILURE 1

#pragma omp declare target
int GlobalVar = 0;
#pragma omp end declare target

int main()
{
  int result = SUCCESS;

  float *LargeData = new float[500000000]; // 2 GB

  #pragma omp target teams num_teams(1) map(to: LargeData[0:500000000]) nowait 
  {
    #pragma omp atomic write
    GlobalVar = 1;    
  }

  #pragma omp target teams num_teams(1) nowait 
  {
    #pragma omp atomic write
    GlobalVar = 2;    
  }

  #pragma omp taskwait

  #pragma omp target update from(GlobalVar)

  result = (GlobalVar == 2 ? SUCCESS : FAILURE);

  delete[] LargeData;

  if(result == SUCCESS) {
    printf("Test finished successfully\n");
    return 0;
  } else if(result == FAILURE) {
    printf("Test failed\n");
    return 1;
  }
}

