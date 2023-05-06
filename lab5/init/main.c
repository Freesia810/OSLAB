#include "put.h"
#include "sched.h"
#include "slub.h"

int start_kernel() {
  const char *msg =
      "ZJU OS LAB 5\n"
      "Student1: Student2:\n";
  puts(msg);

  slub_init();
  task_init();

  // 设置第一次时钟中断
  asm volatile("ecall");

  dead_loop();

  return 0;
}
