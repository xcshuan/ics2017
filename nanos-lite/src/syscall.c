#include "common.h"
#include "syscall.h"

extern char _end;

ssize_t _sys_write(int fd, const void *buf, size_t count){
	Log("sys_write!");
	uintptr_t i = 0;
	if(fd == 1 || fd == 2){
		for(; count > 0;count--){
			_putc(((char *)buf)[i]);
			i++;
		}
		return i;
	}
	return -1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  Log("a[0] = %d",a[0]);
  switch (a[0]) {
	  case SYS_none:r->eax = 1;
					break;
	  case SYS_exit:_halt(a[1]);
					break;
	  case SYS_write:r->eax = _sys_write(a[1],(void *)a[2],a[3]);
					 break;
	  case SYS_brk:r->eax = 0;
				   break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
