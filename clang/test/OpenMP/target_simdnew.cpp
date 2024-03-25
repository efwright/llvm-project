// Test host codegen.
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK1
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK1
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK3
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK3
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK5
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK5
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK7
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK7

// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK9
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK9
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK11
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK11
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK13
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK13
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK15
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK15

// Test target codegen - host bc file has to be created first.
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK17
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK17
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm-bc %s -o %t-x86-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK19
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK19
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK21
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK21
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm-bc %s -o %t-x86-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK23
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK23

// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK9
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK9
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm-bc %s -o %t-x86-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK11
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -fopenmp-version=45 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK11
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm-bc %s -o %t-ppc-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK13
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -std=c++11 -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -triple powerpc64le-unknown-unknown -fopenmp-targets=powerpc64le-ibm-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-ppc-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK13
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm-bc %s -o %t-x86-host.bc
// RUN: %clang_cc1 -verify -Wno-vla -fopenmp-simd -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-llvm %s -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK15
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -std=c++11 -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -emit-pch -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -o %t %s
// RUN: %clang_cc1 -fopenmp-simd -DOMP5 -x c++ -triple i386-unknown-unknown -fopenmp-targets=i386-pc-linux-gnu -std=c++11 -fopenmp-is-target-device -fopenmp-host-ir-file-path %t-x86-host.bc -include-pch %t -verify -Wno-vla %s -emit-llvm -o - | FileCheck -allow-deprecated-dag-overlap  %s --check-prefix=CHECK15

// expected-no-diagnostics
#ifndef HEADER
#define HEADER



// We have 8 target regions, but only 7 that actually will generate offloading
// code, only 6 will have mapped arguments, and only 4 have all-constant map
// sizes.



// Check target registration is registered as a Ctor.


template<typename tx, typename ty>
struct TT{
  tx X;
  ty Y;
};

long long get_val() { return 0; }

int foo(int n) {
  int a = 0;
  short aa = 0;
  float b[10];
  float bn[n];
  double c[5][10];
  double cn[5][n];
  TT<long long, char> d;

  #pragma omp target parallel for simd nowait
  for (int i = 3; i < 32; i += 5) {
  }

  long long k = get_val();
  #pragma omp target parallel for simd if(target: 0) linear(k : 3) schedule(dynamic)
  for (int i = 10; i > 1; i--) {
    a += 1;
  }


  int lin = 12;
  #pragma omp target parallel for simd if(target: 1) linear(lin, a : get_val())
  for (unsigned long long it = 2000; it >= 600; it-=400) {
    aa += 1;
  }




  #pragma omp target parallel for simd if(target: n>10)
  for (short it = 6; it <= 20; it-=-4) {
    a += 1;
    aa += 1;
  }

  // We capture 3 VLA sizes in this target region





  // The names below are not necessarily consistent with the names used for the
  // addresses above as some are repeated.










  #pragma omp target parallel for simd if(target: n>20) schedule(static, a)
  for (unsigned char it = 'z'; it >= 'a'; it+=-1) {
    a += 1;
    b[2] += 1.0;
    bn[3] += 1.0;
    c[1][2] += 1.0;
    cn[1][3] += 1.0;
    d.X += 1;
    d.Y += 1;
  }

  return a;
}

// Check that the offloading functions are emitted and that the arguments are
// correct and loaded correctly for the target regions in foo().




// Create stack storage and store argument in there.

// Create stack storage and store argument in there.

// Create stack storage and store argument in there.

// Create local storage for each capture.



// To reduce complexity, we're only going as far as validating the signature of the outlined parallel function.

template<typename tx>
tx ftemplate(int n) {
  tx a = 0;
  short aa = 0;
  tx b[10];

  #pragma omp target parallel for simd if(target: n>40)
  for (long long i = -10; i < 10; i += 3) {
    a += 1;
    aa += 1;
    b[2] += 1;
  }

  return a;
}

static
int fstatic(int n) {
  int a = 0;
  short aa = 0;
  char aaa = 0;
  int b[10];

  #pragma omp target parallel for simd if(target: n>50)
  for (unsigned i=100; i<10; i+=10) {
    a += 1;
    aa += 1;
    aaa += 1;
    b[2] += 1;
  }

  return a;
}

struct S1 {
  double a;

  int r1(int n){
    int b = n+1;
    short int c[2][n];

#ifdef OMP5
    #pragma omp target parallel for simd if(n>60) nontemporal(a)
#else
    #pragma omp target parallel for simd if(target: n>60)
#endif // OMP5
    for (unsigned long long it = 2000; it >= 600; it -= 400) {
      this->a = (double)b + 1.5;
      c[1][1] = ++a;
    }

    return c[1][1] + (int)b;
  }
};

int bar(int n){
  int a = 0;

  a += foo(n);

  S1 S;
  a += S.r1(n);

  a += fstatic(n);

  a += ftemplate<int>(n);

  return a;
}



// We capture 2 VLA sizes in this target region


// The names below are not necessarily consistent with the names used for the
// addresses above as some are repeated.



















// Check that the offloading functions are emitted and that the arguments are
// correct and loaded correctly for the target regions of the callees of bar().

// Create local storage for each capture.
// Store captures in the context.



// To reduce complexity, we're only going as far as validating the signature of the outlined parallel function.


// Create local storage for each capture.
// Store captures in the context.




// To reduce complexity, we're only going as far as validating the signature of the outlined parallel function.

// Create local storage for each capture.
// Store captures in the context.



// To reduce complexity, we're only going as far as validating the signature of the outlined parallel function.


#endif

