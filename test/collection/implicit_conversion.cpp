// RUN: find-type -config=%S/conf.json %s -- | FileCheck %s
#pragma clang diagnostic ignored "-Wunused-comparison"

typedef double scalar;

class versionNumber {
private:	
  scalar versionNumber_;
  int index_;
public:
	versionNumber(const scalar num) : versionNumber_(num), index_(1) {}
	versionNumber(const char* a, const scalar num=10.0f) : versionNumber_(num),	index_(1) {}
	
  bool operator==(const versionNumber& vn) {
		return index_ == vn.index_;
	}
	
  versionNumber operator*(const versionNumber& vn) {
		return versionNumber(versionNumber_*vn.versionNumber_);
	}
};

class X {
 public:
  X(scalar a);
  bool operator==(const X& other);
};

void f() {
  X a(1.0);
  // CHECK: [[@LINE+1]]:12: [ImplicitConversion]
  if (a == 1.0) {
  }
  // CHECK: [[@LINE+1]]:12: [ImplicitConversion]
  if (a == 2 * 2) {
  }
}

double ad = 1.0; float af = 1.0f; scalar as = 10.0; scalar bs = 1.5;
void container_scalar() {  
  versionNumber vn(1.0);
  // CHECK: [[@LINE+1]]:9: [ImplicitConversion]
  vn == 2;
  // CHECK: [[@LINE+1]]:9: [ImplicitConversion]
  vn == 2 * 10;
  // CHECK: [[@LINE+1]]:9: [ImplicitConversion]
  vn == ad;
  // CHECK: [[@LINE+1]]:9: [ImplicitConversion]
  vn == af;
  // CHECK: [[@LINE+1]]:9: [ImplicitConversion]
  vn == af * ad;
  // CHECK: [[@LINE+1]]:9: [ImplicitConversion]
  vn == !ad;
  // CHECK: [[@LINE+1]]:9: [ImplicitConversion]
  vn == !(af * ad);

  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == scalar(2);
  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == scalar(af);
  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == scalar(af * af);
  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == as;
  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == as * bs;
  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == (as * as * as / as);
  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == (as * (as * as)/as);
  // CHECK-NOT: [[@LINE+1]]:9: [ImplicitConversion]
  vn == (af * (af * as)/af);
}

int main() {
  return 1;
}
