#include "sched.h"

#include "buddy.h"
#include "put.h"
#include "rand.h"
#include "slub.h"

#define offset (0xffffffe000000000 - 0x80000000)

struct task_struct *current;
struct task_struct *task[NR_TASKS];
long PRIORITY_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};
long COUNTER_INIT_COUNTER[NR_TASKS] = {0, 1, 2, 3, 4};

int task_init_done = 0;
extern unsigned long long _end;
extern uint64_t rodata_start;
extern uint64_t data_start;

extern void init_epc(void);
extern void __switch_to(struct task_struct *current, struct task_struct *next);

void task_init(void) {
  puts("task init...\n");

  // initialize task[0]
  current = (struct task_struct*)alloc_pages(1);
  
  current->state = TASK_RUNNING;
  current->counter = COUNTER_INIT_COUNTER[0];
  current->priority = PRIORITY_INIT_COUNTER[0];
  current->blocked = 0;
  current->pid = 0;
  task[0] = current;
  task[0]->thread.sp = (unsigned long long)task[0] + PAGE_SIZE;
  task[0]->thread.ra = (unsigned long long)&init_epc;
  task[0]->mm.satp = (uint64_t)(0x8000000000000000 + (uint64_t)((task_paging_init()) >> 12));
  //task[0]->mm.vm = NULL;
  task[0]->mm.vm = kmalloc(sizeof(struct vm_area_struct));
  INIT_LIST_HEAD(&(task[0]->mm.vm->vm_list));
  

  // set other 4 tasks
  for (int i = 1; i <= 4; ++i) {
    struct task_struct* taskPt = (struct task_struct*)alloc_pages(1);
    taskPt->state = TASK_RUNNING;
    taskPt->counter = COUNTER_INIT_COUNTER[i];
    taskPt->priority = PRIORITY_INIT_COUNTER[i];
    taskPt->blocked = 0;
    taskPt->pid = i;
    task[i] = taskPt;
    task[i]->thread.sp = (unsigned long long)task[i] + PAGE_SIZE;
    task[i]->thread.ra = (unsigned long long)&init_epc;
    task[i]->mm.satp = (uint64_t)(0x8000000000000000 + (uint64_t)((task_paging_init()) >> 12));
    //task[i]->mm.vm = NULL;
    task[i]->mm.vm = kmalloc(sizeof(struct vm_area_struct));
    INIT_LIST_HEAD(&(task[i]->mm.vm->vm_list));

    printf("[PID = %d] Process Create Successfully!\n", task[i]->pid);
  }
  task_init_done = 1;
}

void do_timer(void) {
  if (!task_init_done) return;
  printf("[*PID = ");
  putint(current->pid, 10);
  printf("] Context Calculation: counter = ");
  putint(current->counter, 10);
  printf(",priority = ");
  putint(current->priority, 10);
  printf("\n");
  
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
    printf("[ ");
    putint(current->pid, 10);
    printf(" -> ");
    putint(task[next]->pid, 10);
    printf(" ] Switch from task ");
    putint(current->pid, 10);
    printf("[");
    putullint(current->thread.sp, 16);
    printf("] to task ");
    putint(task[next]->pid, 10);
    printf("[");
    putullint(task[next], 16);
    printf("], prio: ");
    putint(task[next]->priority, 10);
    printf(", counter: ");
    putint(task[next]->counter, 10);
    printf("\n");
  }
  switch_to(task[next]);
}

void switch_to(struct task_struct *next) {
  if(current->pid != next->pid)
  {
    struct task_struct* prev = current;
    
    current = next;
    //更新satp
    asm("ld t0, %0" : : "m"(next->mm.satp));
    asm("csrw satp, t0");
    asm("sfence.vma");
    __switch_to(prev, next);
  }
}

void dead_loop(void) {
  while (1) {
    if (current->pid) {
      mmap(0, 0x1000, 7, 0, 0, 0);
      int *a = (int *)(0x0000);
      *a = 1;
      printf("\033[32m[CHECK] page store OK\033[0m\n\n");

      mmap(0x1000, 0x9000, 7, 0, 0, 0);
      a = (int *)(0x3000);
      int b = (*a);
      printf("\033[32m[CHECK] page load OK\033[0m\n\n");

      while (1)
        ;
    }
    continue;
  }
}

void *mmap(void *__addr, size_t __len, int __prot, int __flags, int __fd, int __offset) {
  /*struct vm_area_struct *temp = kmalloc(sizeof(struct vm_area_struct));
  temp->vm_start = (unsigned long)((unsigned long)((uint64_t)__addr >> 12) << 12);
  temp->vm_end = temp->vm_start + ((__len - 1 ) / PAGE_SIZE + 1) * PAGE_SIZE;
  temp->vm_flags = __prot;
  temp->next = NULL;

  if(current->mm.vm == NULL) {
    current->mm.vm = temp;
  }
  else if (temp->vm_start <= current->mm.vm->vm_start)
  {
    temp->next = current->mm.vm;
    current->mm.vm = temp;
  }
  else {
    //非空
    struct vm_area_struct* node;
    for(node = current->mm.vm; node->next != NULL; node = node->next) {
      if(node->vm_start <= temp->vm_start && node->next->vm_start >= temp->vm_start) {
        temp->next = node->next;
        node->next = temp;
      }
    }
    if(node->vm_start < temp->vm_start) {
      node->next = temp;
    }
  }
  

  return __addr;*/
  


  
  struct vm_area_struct *temp = kmalloc(sizeof(struct vm_area_struct));
  temp->vm_start = (unsigned long)((unsigned long)((uint64_t)__addr >> 12) << 12);
  temp->vm_end = temp->vm_start + ((__len - 1 ) / PAGE_SIZE + 1) * PAGE_SIZE;
  temp->vm_flags = __prot;


  if(list_empty(&(current->mm.vm->vm_list))) {
    list_add_tail(&(temp->vm_list), &(current->mm.vm->vm_list));
  }
  else if (temp->vm_start <= current->mm.vm->vm_start) {
    list_add(&(temp->vm_list), &(current->mm.vm->vm_list));
  }
  else {
    struct list_head *pos;

    struct vm_area_struct* last = list_last_entry(&(current->mm.vm->vm_list), struct vm_area_struct, vm_list);
    if(last->vm_start <= temp->vm_start){
      list_add_tail(&(temp->vm_list), &(current->mm.vm->vm_list));
    }
    else{
      list_for_each(pos, &(current->mm.vm->vm_list)){
      struct vm_area_struct* cur = list_entry(pos, struct vm_area_struct, vm_list);
      struct vm_area_struct* next = list_entry(pos->next, struct vm_area_struct, vm_list);

        if(cur->vm_start <= temp->vm_start && next->vm_start >= temp->vm_start) {
          list_add_behind(&(temp->vm_list), pos);
        }
      }
    }
  }
  
  

  return __addr;
}