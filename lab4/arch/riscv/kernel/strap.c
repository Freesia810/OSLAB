#include "put.h"
#include "sched.h"

typedef unsigned long long uint64_t;

#define INSTRUCTION_PAGE_FAULT (uint64_t)12
#define LOAD_PAGE_FAULT (uint64_t)13
#define STORE_PAGE_FAULT (uint64_t)15

int count = 0;
void handler_s(uint64_t cause) {
  if (cause >> 63) {                // interrupt
    if (((cause << 1) >> 1) == 5) { // supervisor timer interrupt
      asm volatile("ecall");
      do_timer();
      count++;
    }
  }

  switch (cause) {
  case INSTRUCTION_PAGE_FAULT:
    puts("Instruction Page Fault\n");
    break;
  case LOAD_PAGE_FAULT:
    puts("Load Page Fault\n");
    break;
  case STORE_PAGE_FAULT:
    puts("Store Page Fault\n");
    break;
  default:
    break;
  }

  return;
}
