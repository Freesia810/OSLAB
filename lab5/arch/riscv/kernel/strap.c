#include "put.h"
#include "riscv.h"
#include "sched.h"
#include "vm.h"
#include "slub.h"

#define offset (0xffffffe000000000 - 0x80000000)

#define CAUSE_FETCH_PAGE_FAULT 12
#define CAUSE_LOAD_PAGE_FAULT 13
#define CAUSE_STORE_PAGE_FAULT 15


void handler_s(uint64_t cause, uint64_t *stack) {
  if (cause >> 63) {                 // interrupt
    if (((cause << 1) >> 1) == 5) {  // supervisor timer interrupt
      asm volatile("ecall");
      do_timer();
    }
  } 
  else {
    uint64_t bad_addr = read_csr(stval);
    uint64_t cur_pgtbl = (uint64_t)((uint64_t)(current->mm.satp - 0x8000000000000000) << 12); //当前页表物理地址
    struct vm_area_struct* valid_area = NULL;
    
    
    struct list_head *pos;
    list_for_each(pos, &(current->mm.vm->vm_list)){
      struct vm_area_struct* cur = list_entry(pos, struct vm_area_struct, vm_list);
      if(cur->vm_start <= bad_addr && bad_addr <= cur->vm_end){
        valid_area = cur;
      }
    }

    
    /*for(struct vm_area_struct* temp = current->mm.vm; temp != NULL; temp = temp->next) {
      if(temp->vm_start <= bad_addr && bad_addr <= temp->vm_end) {
        valid_area = temp;
      }
    }*/
    
    
    if(cause == (uint64_t)CAUSE_FETCH_PAGE_FAULT) {
      printf("%llx CAUSE_FETCH_PAGE_FAULT\n", bad_addr);

      if(valid_area != NULL && (valid_area->vm_flags & PERM_X)) {
        uint64_t sz = (valid_area->vm_end - valid_area->vm_start);
        uint64_t pa = (uint64_t)kmalloc(sz) - offset;
        create_mapping((uint64_t*)cur_pgtbl, (uint64_t)valid_area->vm_start, pa, sz, PERM_X | PERM_W | PERM_R);
      }
      else {
        printf("Invalid vm area in page fault\n");
      }
    }
    else if(cause == (uint64_t)CAUSE_LOAD_PAGE_FAULT) {
      printf("%llx CAUSE_LOAD_PAGE_FAULT\n", bad_addr);

      if(valid_area != NULL && (valid_area->vm_flags & PERM_R)) {
        uint64_t sz = (valid_area->vm_end - valid_area->vm_start);
        uint64_t pa = (uint64_t)kmalloc(sz) - offset;
        create_mapping((uint64_t*)cur_pgtbl, (uint64_t)valid_area->vm_start, pa, sz, PERM_X | PERM_W | PERM_R);
      }
      else {
        printf("Invalid vm area in page fault\n");
        asm("ld t0, 128(sp)");
        asm("addi t0, t0, 4");
        asm("sd t0, 128(sp)");
      }
    }
    else if(cause == (uint64_t)CAUSE_STORE_PAGE_FAULT) {
      printf("%llx CAUSE_STORE_PAGE_FAULT\n", bad_addr);

      if(valid_area != NULL && (valid_area->vm_flags & PERM_W)) {
        uint64_t sz = (valid_area->vm_end - valid_area->vm_start);
        uint64_t pa = (uint64_t)kmalloc(sz) - offset;
        create_mapping((uint64_t*)cur_pgtbl, (uint64_t)valid_area->vm_start, pa, sz, PERM_X | PERM_W | PERM_R);
      }
      else {
        printf("Invalid vm area in page fault\n");
        asm("ld t0, 128(sp)");
        asm("addi t0, t0, 4");
        asm("sd t0, 128(sp)");
      }
    }
  }
  return;
}
