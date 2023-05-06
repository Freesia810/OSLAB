#include "vm.h"
#include "put.h"
#include "rand.h"

#define offset (0xffffffe000000000 - 0x80000000)
#define KERNEL_TASK_START 0x80210000


typedef unsigned long long uint64_t;

struct task_struct *current;
struct task_struct *task[NR_TASKS];

long PRIORITY_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};
long COUNTER_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};

int task_init_done = 0;
extern uint64_t page_num;
extern uint64_t _end;

extern void init_epc(void);
extern void __switch_to(struct task_struct *current, struct task_struct *next);

extern uint64_t user_paging_init();

void task_init(void) {
  puts("task init...\n");

  // initialize task[0]
  current = (struct task_struct*)(KERNEL_TASK_START + offset);
  current->state = TASK_RUNNING;
  current->counter = COUNTER_INIT_COUNTER[0];
  current->priority = PRIORITY_INIT_COUNTER[0];
  current->blocked = 0;
  current->pid = 0;
  task[0] = current;
  task[0]->thread.sp = (unsigned long long)task[0] + TASK_SIZE;
  task[0]->thread.ra = (unsigned long long)&init_epc;
  task[0]->sscratch = (unsigned long long)task[0] + TASK_SIZE;
  task[0]->satp = (uint64_t)(0x8000000000000000 + (uint64_t)((user_paging_init()) >> 12));


  // set other 4 tasks
  for (int i = 1; i <= 4; ++i) {
    struct task_struct* taskPt = (struct task_struct*)(KERNEL_TASK_START + i * TASK_SIZE + offset);
    taskPt->state = TASK_RUNNING;
    taskPt->counter = COUNTER_INIT_COUNTER[i];
    taskPt->priority = PRIORITY_INIT_COUNTER[i];
    taskPt->blocked = 0;
    taskPt->pid = i;
    task[i] = taskPt;
    task[i]->thread.sp = (unsigned long long)task[i] + TASK_SIZE;
    task[i]->thread.ra = (unsigned long long)&init_epc;
    task[i]->sscratch = (unsigned long long)task[i] + TASK_SIZE;
    task[i]->satp = (uint64_t)(0x8000000000000000 + (uint64_t)((user_paging_init()) >> 12));

    puts("[PID = ");
    putint(task[i]->pid, 10);
    puts("] Process Create Successfully!\n");
  }

  asm("ld t0, %0" : : "m"(task[0]->sscratch));
  asm("csrw sscratch, t0");
  asm("ld t0, %0" : : "m"(task[0]->satp));
  asm("csrw satp, t0");
  asm("sfence.vma");
  task_init_done = 1;
}

void do_timer(void) {
  if (!task_init_done) return;
  puts("[*PID = ");
  putint(current->pid, 10);
  puts("] Context Calculation: counter = ");
  putint(current->counter, 10);
  puts(",priority = ");
  putint(current->priority, 10);
  puts("\n");
  
  current->counter--;
  if(current->counter < 0)
  {
    schedule();
  }
  
}

void schedule(void) {
  unsigned char next;

  int minLeftTime = __INT_MAX__;
  int minLeftTask = -1;

  for(int i = 4; i > 0; i--)
  {
    if(task[i]->counter < minLeftTime && task[i]->counter >= 0)
    {
      minLeftTime = task[i]->counter;
      minLeftTask = i;
    }
  }

  if(minLeftTask == -1)
  {
    task[1]->counter = rand();
    task[1]->priority = rand();
    task[2]->counter = rand();
    task[2]->priority = rand();
    task[3]->counter = rand();
    task[3]->priority = rand();
    task[4]->counter = rand();
    task[4]->priority = rand();

    schedule();
    return;
  }
  else{
    next = (unsigned char)minLeftTask;
  }

  if (current->pid != task[next]->pid) {
    puts("[ ");
    putint(current->pid, 10);
    puts(" -> ");
    putint(task[next]->pid, 10);
    puts(" ] Switch from task ");
    putint(current->pid, 10);
    puts("[");
    putullint(current->thread.sp, 16);
    puts("] to task ");
    putint(task[next]->pid, 10);
    puts("[");
    putullint(task[next], 16);
    puts("], prio: ");
    putint(task[next]->priority, 10);
    puts(", counter: ");
    putint(task[next]->counter, 10);
    puts("\n");
  }
  switch_to(task[next]);
}

void switch_to(struct task_struct *next) {
  if(current->pid != next->pid)
  {
    struct task_struct* prev = current;
    current = next;
    __switch_to(prev, next);
  }
}

void dead_loop(void) {
  while (1) {
    continue;
  }
}
