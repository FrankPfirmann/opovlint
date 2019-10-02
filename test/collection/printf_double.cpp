// RUN: find-type -config=%S/conf_double.json %s -- | FileCheck %s --check-prefix=CHECK-DOUBLE

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
  double b = 1.0;
  scalar c = 1.0;
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::printf("printing %f", b);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::printf("printing %f %d", b, 3);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::printf("printing %f %f", c, b);
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  printf();
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  scf(b);
  // CHECK-DOUBLE-NOT [[@LINE+1]]:3: [Printf]
  std::printf("printing %f", c);

  FILE* file;
  file = fopen("out", "w");
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f", b);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f %d", b, 3);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f %f", c, b);
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  fprintf();
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  std::fprintf(file, "printing %f", c);

  char buffer[10];
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f", b);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f %d", b, 3);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f %f", c, b);
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  sprintf();
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  std::sprintf(buffer, "printing %f", c);

  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f", b);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f %d", b, 3);
  // CHECK-DOUBLE: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f %f", c, b);
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  snprintf();
  // CHECK-DOUBLE-NOT: [[@LINE+1]]:3: [Printf]
  std::snprintf(buffer, 10, "printing %f", c);
}
