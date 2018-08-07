#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
  if(id_dest->width == 1){
	  rtl_sext(&id_dest->val, &id_dest->val,id_dest->width);
  }
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  //TODO();
  rtl_pop(&t0);
  operand_write(id_dest,&t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
	t0 = cpu.esp;
	for(int i = R_EAX;i <= R_EDI; i++){
	  if(i != R_ESP){
		  rtl_push(&cpu.gpr[i]._32);
	  }
	  else{
		  rtl_push(&t0);
	  }
	}
  print_asm("pusha");
}

make_EHelper(popa) {
	for(int i = R_EDI;i >= R_EAX; i--){
		if(i != R_ESP){
			rtl_pop(&cpu.gpr[i]._32);
					}
		else{
			rtl_pop(&t0);
		}
  }

  print_asm("popa");
}

make_EHelper(leave) {
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {

 if (decoding.is_operand_size_16) {
 
	 rtl_lr(&t0, R_AX, 2);
	 if ((int32_t)(int16_t)(uint16_t)t0 < 0) {
		 rtl_addi(&t1, &tzero, 0xffff);
		 rtl_sr(R_DX, 2, &t1);
	 }
	 else {
		 rtl_sr(R_DX, 2, &tzero);
	 }
 }
 else {
	 rtl_lr(&t0, R_EAX, 4);
	 if ((int32_t)t0 < 0) {
		 rtl_addi(&t1, &tzero, 0xffffffff);
		 rtl_sr(R_EDX, 4, &t1);
	 }
	 else {
		 rtl_sr(R_EDX, 4, &tzero);
	 }
 }
  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
	if (decoding.is_operand_size_16) {
		rtl_lr(&t0, R_AL, 1);
		t0 = (int16_t)(int8_t)(uint8_t)t0;
		rtl_sr(R_AX, 2, &t0);
	}
	else {
		rtl_lr(&t0, R_AX, 2);
		t0 = (int32_t)(int16_t)(uint16_t)t0;
		rtl_sr(R_EAX, 4, &t0);
	}

	
	print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
