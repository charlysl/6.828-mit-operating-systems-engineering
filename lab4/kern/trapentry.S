/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(vector0, 0);
TRAPHANDLER_NOEC(vector1, 1);
TRAPHANDLER_NOEC(vector2, 2);
TRAPHANDLER_NOEC(vector3, 3);
TRAPHANDLER_NOEC(vector4, 4);
TRAPHANDLER_NOEC(vector5, 5);
TRAPHANDLER_NOEC(vector6, 6);
TRAPHANDLER_NOEC(vector7, 7);
TRAPHANDLER(vector8, 8);
TRAPHANDLER(vector10, 10);
TRAPHANDLER(vector11, 11);
TRAPHANDLER(vector12, 12);
TRAPHANDLER(vector13, 13);
TRAPHANDLER(vector14, 14);
TRAPHANDLER_NOEC(vector16, 16);
TRAPHANDLER(vector17, 17);
TRAPHANDLER_NOEC(vector18, 18);
TRAPHANDLER(vector19, 19);
TRAPHANDLER_NOEC(vector_SYSCALL, T_SYSCALL);

// Hardware IRQ numbers. We receive these as (IRQ_OFFSET+IRQ_WHATEVER)
TRAPHANDLER_NOEC(vector_IRQ_TIMER,    IRQ_OFFSET+IRQ_TIMER)
TRAPHANDLER_NOEC(vector_IRQ_KBD,      IRQ_OFFSET+IRQ_KBD)
TRAPHANDLER_NOEC(vector_IRQ_SERIAL,   IRQ_OFFSET+IRQ_SERIAL)
TRAPHANDLER_NOEC(vector_IRQ_SPURIOUS, IRQ_OFFSET+IRQ_SPURIOUS)
TRAPHANDLER_NOEC(vector_IRQ_IDE,      IRQ_OFFSET+IRQ_IDE)
TRAPHANDLER_NOEC(vector_IRQ_ERROR,    IRQ_OFFSET+IRQ_ERROR)


/*
 * Lab 3: Your code here for _alltraps
 */

.globl _alltraps
_alltraps:
	# 1. push values to make the stack look like a struct Trapframe
	pushl %ds
	pushl %es
	#pushl %fs    not in struct Trapframe
	#pushl %gs    idem
	pushal

	# Set up data per-cpu segments
	#movw $(1<<3), %ax	# kernel code
	# 2. load GD_KD into %ds and %es
	movw $(GD_KD), %ax
	movw %ax, %ds
	movw %ax, %es
	#movw $(3<<3), %ax	# kernel per-cpu data
	#movw %ax, %fs
	#movw %ax, %gs
	
	# Call trap(tf), where tf=%esp
	pushl %esp
	call trap	# never returns, back to user via env_run()

