#include "defs.h"
#include "print.h"
extern int test();

int main() {
  puts("Hello riscv \n");
  // Change the number to your stu_id.
  put_num(3);
  puts("\n");
  test();
  return 0;
}
