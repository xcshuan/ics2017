#include "common.h"
#include "syscall.h"
#include "fs.h"

extern char _end;

ssize_t _sys_write(int fd, const void *buf, size_t count){
	Log("sys_write!count = %d",count);
	ssize_t i = 0;
	if(fd == 1 || fd == 2){
		for(;i < count;i++){
			_putc(((char *)buf)[i]);
		}
		return count;
	}
	return -1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);

  //Log("a[0] = %d",a[0]);
  switch (a[0]) {
	  case SYS_none:r->eax = 1;
					break;
	  case SYS_exit:_halt(a[1]);
					break;
	  case SYS_write:r->eax = fs_write(a[1],(void *)a[2],a[3]);
					 break;
	  case SYS_open:r->eax = fs_open((char *)a[1],a[2],a[3]);
					break;
	  case SYS_read:r->eax = fs_read(a[1], (char *)a[2],a[3]);
					break;
	  case SYS_lseek:r->eax = fs_lseek(a[1],a[2],a[3]);
					 break;
	  case SYS_close:r->eax = fs_close(a[1]);
					 break;
	  case SYS_brk:r->eax = 0;
				   break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
