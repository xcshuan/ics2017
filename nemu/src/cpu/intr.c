#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  Log("it is raise_intr,NO = %d",NO);
  union{
	  GateDesc gd;
	  struct{uint32_t lo,hi;};
  }item;
  vaddr_t addr;
  addr = 8 * NO + cpu.idtr.base;
  item.lo = vaddr_read(addr,4);
  item.hi = vaddr_read(addr + 4, 4);

  decoding.is_jmp = 1;
  decoding.jmp_eip = (item.gd.offset_15_0 & 0xffff) | ((item.gd.offset_31_16 &0xffff)<<16);
  rtl_push(&cpu.eflags.eflags_init);
  t0 = cpu.cs;
  rtl_push(&t0);
  rtl_push(&ret_addr);

  cpu.eflags.IF = 0;
}

void dev_raise_intr() {
}
