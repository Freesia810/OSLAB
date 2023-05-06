#include "put.h"
#include "sched.h"

#define TEST 1

extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;

int start_kernel() {
  const char *msg = "ZJU OS LAB 4\n"
                    "Student1: Student2:\n";
  puts(msg);

  // 设置第一次时钟中断
  asm volatile("ecall");

  task_init();

  #if TEST
  unsigned long long* testA = &rodata_start;
  *testA = 64;
  void(*fp)(void) = (void(*)(void))&data_start;
  (*fp)();
  #endif

  dead_loop();

  return 0;
}
