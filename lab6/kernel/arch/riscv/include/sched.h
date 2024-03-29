#ifndef _SCHED_H
#define _SCHED_H

#define TASK_SIZE (4096)
#define THREAD_OFFSET (5 * 0x08)

typedef unsigned long long uint64_t;

#ifndef __ASSEMBLER__

/* task的最大数量 */
#define NR_TASKS 64

#define FIRST_TASK (task[0])
#define LAST_TASK (task[NR_TASKS - 1])

/* 定义 task 的状态，本 Lab 中 task 只需要一种状态。*/
#define TASK_RUNNING 0

/* 当前进程 */
extern struct task_struct *current;

/* 进程指针数组 */
extern struct task_struct *task[NR_TASKS];

/* 进程状态段数据结构 */
struct thread_struct {
  unsigned long long ra;
  unsigned long long sp;
  unsigned long long s0;
  unsigned long long s1;
  unsigned long long s2;
  unsigned long long s3;
  unsigned long long s4;
  unsigned long long s5;
  unsigned long long s6;
  unsigned long long s7;
  unsigned long long s8;
  unsigned long long s9;
  unsigned long long s10;
  unsigned long long s11;
};

/* 进程数据结构 */
struct task_struct {
  long state;     // 进程状态 Lab3中进程初始化时置为TASK_RUNNING
  long counter;   // 运行剩余时间
  long priority;  // 运行优先级 1最高 5最低
  long blocked;
  long pid;  // 进程标识符

  uint64_t sscratch; // 保存 sscratch
  uint64_t satp;     // 保存该进程对应的页表

  // Above Size Cost: 56 bytes
  struct thread_struct thread;  // 该进程状态段
};

/* 进程初始化 创建四个dead_loop进程 */
void task_init(void);

/* 在时钟中断处理中被调用 */
void do_timer(void);

/* 调度程序 */
void schedule(void);

/* 切换当前任务 current 到下一个任务 next */
void switch_to(struct task_struct *next);

/* 死循环 */
void dead_loop(void);

#endif

#endif
