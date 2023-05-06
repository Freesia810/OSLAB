#include "stdio.h"
#include "syscall.h"

register void *current_sp __asm__("sp");

static inline long getpid() {
  long ret;
  asm volatile("li a7, %1\n"
               "ecall\n"
               "mv %0, a0\n"
               : "+r"(ret)
               : "i"(SYS_GETPID)
               : "a7");
  return ret;
}

int main() {
  while (1) {
    printf("[User] pid: %ld, sp is %llx\n", getpid(), current_sp);
    for (unsigned int i = 0; i < 0xFFFFFF; i++)
      ;
  }

  return 0;
}
