#include "put.h"
#include "sched.h"

extern unsigned long long _end;

int start_kernel() {
  const char *msg = "ZJU OS LAB 6\n"
                    "Student1:\n";
  
  puts(msg);
  task_init();

  // 设置第一次时钟中断
  asm volatile("ecall");

  dead_loop();

  return 0;
}
