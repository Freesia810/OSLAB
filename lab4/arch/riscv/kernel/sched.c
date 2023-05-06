#include "sched.h"
#include "put.h"
#include "rand.h"

#define offset (0xffffffe000000000 - 0x80000000)
#define NR_TASKS 64
#define Kernel_Page 0x80210000
#define LOW_MEMORY 0x80211000

struct task_struct *current;
struct task_struct *task[NR_TASKS];
long PRIORITY_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};
long COUNTER_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};

extern void init_epc(void);
extern void __switch_to(struct task_struct *current, struct task_struct *next);

int task_init_done = 0;
void task_init(void) {
  puts("task init...\n");

  // initialize task[0]
  current = (struct task_struct*)(Kernel_Page + offset);
  current->state = TASK_RUNNING;
  current->counter = COUNTER_INIT_COUNTER[0];
  current->priority = PRIORITY_INIT_COUNTER[0];
  current->blocked = 0;
  current->pid = 0;
  task[0] = current;
  task[0]->thread.sp = (unsigned long long)task[0] + TASK_SIZE;
  task[0]->thread.ra = (unsigned long long)&init_epc;

  // set other 4 tasks
  for (int i = 1; i <= 4; ++i) {
    struct task_struct* taskPt = (struct task_struct*)(LOW_MEMORY + (i - 1) * 0x1000 + offset);
    taskPt->state = TASK_RUNNING;
    taskPt->counter = COUNTER_INIT_COUNTER[i];
    taskPt->priority = PRIORITY_INIT_COUNTER[i];
    taskPt->blocked = 0;
    taskPt->pid = i;
    task[i] = taskPt;
    task[i]->thread.sp = (unsigned long long)task[i] + TASK_SIZE;
    task[i]->thread.ra = (unsigned long long)&init_epc;

    puts("[PID = ");
    puti(task[i]->pid);
    puts("] Process Create Successfully!\n");
  }
  task_init_done = 1;
}

void do_timer(void) {
  if (!task_init_done) return;
  puts("[*PID = ");
  puti(current->pid);
  puts("] Context Calculation: counter = ");
  puti(current->counter);
  puts(",priority = ");
  puti(current->priority);
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
    puti(current->pid);
    puts(" -> ");
    puti(task[next]->pid);
    puts(" ] Switch from task ");
    puti(current->pid);
    puts("[");
    putullHex(current->thread.sp);
    puts("] to task ");
    puti(task[next]->pid);
    puts("[");
    putullHex(task[next]);
    puts("], prio: ");
    puti(task[next]->priority);
    puts(", counter: ");
    puti(task[next]->counter);
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
