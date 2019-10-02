// RUN: find-type -config=%S/conf.json %s -- | FileCheck %s
#pragma clang diagnostic ignored "-Wunused-value"
#include<cstdio>
typedef double scalar;
void printf(){
}
void fprintf(){
}
void sprintf(){
}
void snprintf(){
}
void scf(scalar a){
}
void function(){
  scalar b = 1.0;
  double c = 3.0;
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::printf("printing %f", b);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::printf("printing %f %d", b, 3);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::printf("printing %f %f", c, b*b);
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  printf();
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  scf(b);
  // CHECK-NOT [[@LINE+1]]:3: [Printf]
  std::printf("printing %f", c);

  FILE* file;
  file = fopen("out", "w");
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f", b);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f %d", b*b, 3);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f %f", c, b);
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  fprintf();
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f", c);

  char buffer[10];
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f", b);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f %d", b*b, 3);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f %f", c, b);
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  sprintf();
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f", c);

  // CHECK: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f", b);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f %d", b*b, 3);
  // CHECK: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f %f", c, b);
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  snprintf();
  // CHECK-NOT: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f", c);
}
