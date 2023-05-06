#include "put.h"
#include "sched.h"

typedef unsigned long long uint64_t;
typedef unsigned long size_t;


size_t sys_write(unsigned int fd, const char* buf, size_t count) {
  size_t i = 0;
  if (fd == 1) {
    for (i = 0; buf[i] != 0 && i < count; i++) {
      *UART16550A_DR = (unsigned char)buf[i];
    }
  }
  return i;
}


void handler_s(uint64_t cause, uint64_t* stack) {
  if (cause >> 63) {                 // interrupt
    if (((cause << 1) >> 1) == 5) {  // supervisor timer interrupt
      asm volatile("ecall");
      do_timer();
    }
  }
  else {
    //exception
    if(cause == (uint64_t)8) {
      //ecall U
      uint64_t* arg0 = &stack[11];
      uint64_t* arg1 = &stack[10];
      uint64_t arg2 = stack[9];
      uint64_t arg3 = stack[8];
      uint64_t arg4 = stack[7];
      uint64_t arg5 = stack[6];
      uint64_t arg6 = stack[5];
      uint64_t arg7 = stack[4];

      if(arg7 == (uint64_t)64) {
        //write
        *arg0 = (uint64_t)sys_write((unsigned int)(*arg0), (char*)(*arg1), (size_t)arg2);
      }
      else if(arg7 == (uint64_t)172) {
        //pid
        *arg0 = (long)(current->pid);
      }
    }
  }
  return;
}
