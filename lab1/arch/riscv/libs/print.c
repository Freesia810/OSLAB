#include "defs.h"
extern struct sbiret sbi_call(uint64_t ext, uint64_t fid, uint64_t arg0,
                              uint64_t arg1, uint64_t arg2, uint64_t arg3,
                              uint64_t arg4, uint64_t arg5);

int puts(char *str) {
  uint64_t i = 0;
  while (str[i] != '\0')
  {
    sbi_call(1,0,str[i],0,0,0,0,0);
    i++;
  }
  
  return 0;
}

int put_num(uint64_t n) {
  int digit = 0;
	uint64_t num = n;
	if (num == 0)
	{
		digit = 1;
	}
	else {
      while (num > 0)
      {
        num = num / 10;
        digit++;
      }
	}

	char str[21];
	str[digit] = '\0';
	for (int i = digit - 1 ; i >= 0; i--)
	{
      int single = n % 10;
      str[i] = single + '0';
      n = n / 10;
	}
	puts(str);
  return 0;
}