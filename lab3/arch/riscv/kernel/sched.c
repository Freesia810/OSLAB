#include "sched.h"

#include "stdio.h"

#define Kernel_Page 0x80210000
#define LOW_MEMORY 0x80211000
#define PAGE_SIZE 4096UL

struct task_struct* current;
struct task_struct* task[NR_TASKS];

extern task_test_done;
extern void __init_sepc(void);

// If next==current,do nothing; else update current and call __switch_to.
void switch_to(struct task_struct* next) {
  if(current->pid != next->pid)
  {
    struct task_struct* prev = current;
    current = next;
    __switch_to(prev, next);
  }
}

int task_init_done = 0;
// initialize tasks, set member variables
void task_init(void) {
  puts("task init...\n");

  // initialize task[0]
  current = (struct task_struct*)Kernel_Page;
  current->state = TASK_RUNNING;
  current->counter = 1;
  current->priority = 5;
  current->blocked = 0;
  current->pid = 0;
  task[0] = current;
  task[0]->thread.sp = (unsigned long long)task[0] + TASK_SIZE;
  task[0]->thread.ra = (unsigned long long)&__init_sepc;

  // set other 4 tasks
  for (int i = 1; i <= LAB_TEST_NUM; ++i) {
    struct task_struct* taskPt = (struct task_struct*)(LOW_MEMORY + (i - 1) * 0x1000);
    taskPt->state = TASK_RUNNING;
    taskPt->counter = 0;
    taskPt->priority = 5;
    taskPt->blocked = 0;
    taskPt->pid = i;
    task[i] = taskPt;
    task[i]->thread.sp = (unsigned long long)task[i] + TASK_SIZE;
    task[i]->thread.ra = (unsigned long long)&__init_sepc;

    printf("[PID = %d] Process Create Successfully!\n", task[i]->pid);
  }
  task_init_done = 1;
}

#ifdef SJF
// simulate the cpu timeslice, which measn a short time frame that gets assigned
// to process for CPU execution
void do_timer(void) {
  if (!task_init_done) return;
  if (task_test_done) return;
  printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n",
         current->pid, current->counter, current->priority);
  // current process's counter -1, judge whether to schedule or go on.
  current->counter--;
  if(current->counter == 0)
  {
    schedule();
  }
}

// Select the next task to run. If all tasks are done(counter=0), set task0's
// counter to 1 and it would assign new test case.
void schedule(void) {
  unsigned char next;

  int minLeftTime = __INT_MAX__;
  int minLeftTask = -1;
  for(int i = LAB_TEST_NUM; i > 0;i--)
  {
    if(task[i]->counter < minLeftTime && task[i]->counter > 0)
    {
      minLeftTime = task[i]->counter;
      minLeftTask = i;
    }
  }

  if(minLeftTask == -1)
  {
    task[0]->counter = 1;
    next = 0;
  }
  else{
    next = (unsigned char)minLeftTask;
  }

  if (current->pid != task[next]->pid) {
    printf(
        "[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, "
        "counter: %d\n",
        current->pid, task[next]->pid, current->pid,
        (unsigned long)current->thread.sp, task[next]->pid,
        (unsigned long)task[next], task[next]->priority, task[next]->counter);
  }
  switch_to(task[next]);
}

#endif

#ifdef PRIORITY

// simulate the cpu timeslice, which measn a short time frame that gets assigned
// to process for CPU execution
void do_timer(void) {
  if (!task_init_done) return;
  if (task_test_done) return;
  printf("[*PID = %d] Context Calculation: counter = %d,priority = %d\n",
         current->pid, current->counter, current->priority);
  // current process's counter -1, judge whether to schedule or go on.
  current->counter--;
  schedule();
}

// Select the next task to run. If all tasks are done(counter=0), set task0's
// counter to 1 and it would assign new test case.
void schedule(void) {
  unsigned char next;
  
  int maxPriority = -1;
  int targetTask = -1;
  int targetLeftTime = __INT_MAX__;
  for(int i = LAB_TEST_NUM; i > 0;i--)
  {
    if(task[i]->counter > 0)
    {
      if(task[i]->priority > maxPriority)
      {
        maxPriority = task[i]->priority;
        targetTask = i;
        targetLeftTime = task[i]->counter;
      }
      else if(task[i]->priority == maxPriority)
      {
        if(task[i]->counter < targetLeftTime)
        {
          targetTask = i;
          targetLeftTime = task[i]->counter;
        }
      }
    }
  }

  if(targetTask == -1)
  {
    task[0]->counter = 1;
    next = 0;
  }
  else{
    next = (unsigned char)targetTask;
  }

  if (current->pid != task[next]->pid) {
    printf(
        "[ %d -> %d ] Switch from task %d[%lx] to task %d[%lx], prio: %d, "
        "counter: %d\n",
        current->pid, task[next]->pid, current->pid,
        (unsigned long)current->thread.sp, task[next]->pid,
        (unsigned long)task[next], task[next]->priority, task[next]->counter);
  }
  switch_to(task[next]);
}

#endif
