#include "defs.h"

struct sbiret sbi_call(uint64_t ext, uint64_t fid, uint64_t arg0, uint64_t arg1,
                       uint64_t arg2, uint64_t arg3, uint64_t arg4,
                       uint64_t arg5) {
  struct sbiret ret;
  uint64_t retError;
  uint64_t retValue;
  __asm__ volatile(
      "mv a7, %[ext]\n"
      "mv a6, %[fid]\n"
      "mv a0, %[arg0]\n"
      "mv a1, %[arg1]\n"
      "mv a2, %[arg2]\n"
      "mv a3, %[arg3]\n"
      "mv a4, %[arg4]\n"
      "mv a5, %[arg5]\n"
      "ecall\n"
      "mv %[retError], a0\n"
      "mv %[retValue], a1"
      : [retError] "=r" (retError), [retValue] "=r" (retValue)
      : [ext] "r" (ext), [fid] "r" (fid), [arg0] "r" (arg0), [arg1] "r" (arg1), [arg2] "r" (arg2), 
      [arg3] "r" (arg3), [arg4] "r" (arg4), [arg5] "r" (arg5)
      : "memory"
  );
  ret.error = retError;
  ret.value = retValue;
  return ret;
}
