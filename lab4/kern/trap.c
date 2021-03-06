#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>

static struct Taskstate ts;

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};

/* Declare TRAPHANDLER functions */
void vector0();
void vector1();
void vector2();
void vector3();
void vector4();
void vector5();
void vector6();
void vector7();
void vector8();
void vector10();
void vector11();
void vector12();
void vector13();
void vector14();
void vector16();
void vector17();
void vector18();
void vector19();
void vector_SYSCALL();

void vector_IRQ_TIMER();
void vector_IRQ_KBD();
void vector_IRQ_SERIAL();
void vector_IRQ_SPURIOUS();
void vector_IRQ_IDE();
void vector_IRQ_ERROR();

static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
		return "Hardware Interrupt";
	return "(unknown trap)";
}


void
trap_init(void)
{
	extern struct Segdesc gdt[];

	// LAB 3: Your code here.
	//cprintf("trap_init entered  vector0 %p\n", vector0);

	SETGATE(idt[0], 0, GD_KT, vector0, 0); 
	SETGATE(idt[1], 0, GD_KT, vector1, 0); 
	SETGATE(idt[2], 0, GD_KT, vector2, 0); 
	SETGATE(idt[T_BRKPT], 0, GD_KT, vector3, 3); 
	SETGATE(idt[4], 0, GD_KT, vector4, 0); 
	SETGATE(idt[5], 0, GD_KT, vector5, 0); 
	SETGATE(idt[6], 0, GD_KT, vector6, 0); 
	SETGATE(idt[7], 0, GD_KT, vector7, 0); 
	SETGATE(idt[8], 0, GD_KT, vector8, 0); 
	SETGATE(idt[10], 0, GD_KT, vector10, 0); 
	SETGATE(idt[11], 0, GD_KT, vector11, 0); 
	SETGATE(idt[12], 0, GD_KT, vector12, 0); 
	SETGATE(idt[13], 0, GD_KT, vector13, 0); 
	SETGATE(idt[14], 0, GD_KT, vector14, 0); 
	SETGATE(idt[16], 0, GD_KT, vector16, 0); 
	SETGATE(idt[17], 0, GD_KT, vector17, 0); 
	SETGATE(idt[18], 0, GD_KT, vector18, 0); 
	SETGATE(idt[19], 0, GD_KT, vector19, 0); 
	SETGATE(idt[T_SYSCALL], 0, GD_KT, vector_SYSCALL, 3); 

	SETGATE(idt[IRQ_OFFSET+IRQ_TIMER],    0, GD_KT, vector_IRQ_TIMER,    0);
	SETGATE(idt[IRQ_OFFSET+IRQ_KBD],      0, GD_KT, vector_IRQ_KBD,      0);
	SETGATE(idt[IRQ_OFFSET+IRQ_SERIAL],   0, GD_KT, vector_IRQ_SERIAL,   0);
	SETGATE(idt[IRQ_OFFSET+IRQ_SPURIOUS], 0, GD_KT, vector_IRQ_SPURIOUS, 0);
	SETGATE(idt[IRQ_OFFSET+IRQ_IDE],      0, GD_KT, vector_IRQ_IDE,      0);
	SETGATE(idt[IRQ_OFFSET+IRQ_ERROR],    0, GD_KT, vector_IRQ_ERROR,    0);


	// Per-CPU setup 
	trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void
trap_init_percpu(void)
{
	// The example code here sets up the Task State Segment (TSS) and
	// the TSS descriptor for CPU 0. But it is incorrect if we are
	// running on other CPUs because each CPU has its own kernel stack.
	// Fix the code so that it works for all CPUs.
	//
	// Hints:
	//   - The macro "thiscpu" always refers to the current CPU's
	//     struct Cpu;
	//   - The ID of the current CPU is given by cpunum() or
	//     thiscpu->cpu_id;
	//   - Use "thiscpu->cpu_ts" as the TSS for the current CPU,
	//     rather than the global "ts" variable;
	//   - Use gdt[(GD_TSS0 >> 3) + i] for CPU i's TSS descriptor;
	//   - You mapped the per-CPU kernel stacks in mem_init_mp()
	//
	// ltr sets a 'busy' flag in the TSS selector, so if you
	// accidentally load the same TSS on more than one CPU, you'll
	// get a triple fault.  If you set up an individual CPU's TSS
	// wrong, you may not get a fault until you try to return from
	// user space on that CPU.
	//
	// LAB 4: Your code here:

	//cprintf("trap_init_percpu  cpu %d\n", cpunum());

	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	//ts.ts_esp0 = KSTACKTOP; 
	//ts.ts_ss0 = GD_KD;
	thiscpu->cpu_ts.ts_esp0 = KSTACKTOP - cpunum() * (KSTKSIZE + KSTKGAP);
	thiscpu->cpu_ts.ts_ss0 = GD_KD;


	// Initialize the TSS slot of the gdt.
	gdt[(GD_TSS0 >> 3) + cpunum()] = SEG16(STS_T32A, (uint32_t) (&thiscpu->cpu_ts),
					sizeof(struct Taskstate), 0);
	gdt[(GD_TSS0 >> 3) + cpunum()].sd_s = 0;

	// Load the TSS selector (like other segment selectors, the
	// bottom three bits are special; we leave them 0)
	ltr(GD_TSS0 + (cpunum() << 3));

	// Load the IDT
	lidt(&idt_pd);

	//cprintf("trap_init_percpu  cpu %d, esp0 %p\n", cpunum(), thiscpu->cpu_ts.ts_esp0);
}

void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p from CPU %d\n", tf, cpunum());
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	// If this trap was a page fault that just happened
	// (so %cr2 is meaningful), print the faulting linear address.
	if (tf == last_tf && tf->tf_trapno == T_PGFLT)
		cprintf("  cr2  0x%08x\n", rcr2());
	cprintf("  err  0x%08x", tf->tf_err);
	// For page faults, print decoded fault error code:
	// U/K=fault occurred in user/kernel mode
	// W/R=a write/read caused the fault
	// PR=a protection violation caused the fault (NP=page not present).
	if (tf->tf_trapno == T_PGFLT)
		cprintf(" [%s, %s, %s]\n",
			tf->tf_err & 4 ? "user" : "kernel",
			tf->tf_err & 2 ? "write" : "read",
			tf->tf_err & 1 ? "protection" : "not-present");
	else
		cprintf("\n");
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	if ((tf->tf_cs & 3) != 0) {
		cprintf("  esp  0x%08x\n", tf->tf_esp);
		cprintf("  ss   0x----%04x\n", tf->tf_ss);
	}
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

static void
trap_dispatch(struct Trapframe *tf)
{
	//cprintf("trap_dispatch\n");
	// Handle processor exceptions.
	// LAB 3: Your code here.

	// Handle spurious interrupts
	// The hardware sometimes raises these because of noise on the
	// IRQ line or other reasons. We don't care.
	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
		cprintf("Spurious interrupt on irq 7\n");
		print_trapframe(tf);
		return;
	}

	// Handle clock interrupts. Don't forget to acknowledge the
	// interrupt using lapic_eoi() before calling the scheduler!
	// LAB 4: Your code here.

	switch (tf->tf_trapno) {
		case T_BRKPT:
			monitor(tf);
			break;
		case T_PGFLT:
			page_fault_handler(tf);	
			break;
		case T_SYSCALL:
			//cprintf("trap_dispatch syscall %d\n", tf->tf_regs.reg_eax);
			tf->tf_regs.reg_eax = syscall(
			 				tf->tf_regs.reg_eax, 
							tf->tf_regs.reg_edx,
							tf->tf_regs.reg_ecx,
							tf->tf_regs.reg_ebx,
							tf->tf_regs.reg_edi,
							tf->tf_regs.reg_esi
			);
			//cprintf("trap_dispatch syscall ret\n");
			return;
		case IRQ_OFFSET+IRQ_TIMER:
			//cprintf("trap_dispatch timer  %d\n", tf->tf_trapno);

			// ack interrupt, otherwise there are no more timer interrupts
			lapic_eoi(); 

			sched_yield();
			panic("trap_dispatch timer");
		case IRQ_OFFSET+IRQ_KBD:
		case IRQ_OFFSET+IRQ_SERIAL:
		case IRQ_OFFSET+IRQ_SPURIOUS:
		case IRQ_OFFSET+IRQ_IDE:
		case IRQ_OFFSET+IRQ_ERROR:
			cprintf("trap_dispatch irq  %d\n", tf->tf_trapno);
			return;
		default:
			// Unexpected trap: The user process or the kernel has a bug.
			print_trapframe(tf);
			if (tf->tf_cs == GD_KT)
				panic("unhandled trap in kernel");
			else {
				env_destroy(curenv);
				return;
			}
	}

	// Unexpected trap: The user process or the kernel has a bug.
	print_trapframe(tf);
	if (tf->tf_cs == GD_KT)
		panic("unhandled trap in kernel");
	else {
		//cprintf("trap_dispatch env_destroy\n");
		env_destroy(curenv);
		return;
	}

}

void
trap(struct Trapframe *tf)
{
	//cprintf("trap frame:\n");
	//print_trapframe(tf);

	// The environment may have set DF and some versions
	// of GCC rely on DF being clear
	asm volatile("cld" ::: "cc");

	// Halt the CPU if some other CPU has called panic()
	extern char *panicstr;
	if (panicstr)
		asm volatile("hlt");

	// Check that interrupts are disabled.  If this assertion
	// fails, DO NOT be tempted to fix it by inserting a "cli" in
	// the interrupt path.
	assert(!(read_eflags() & FL_IF));

	if ((tf->tf_cs & 3) == 3) {
		// Trapped from user mode.
		// Acquire the big kernel lock before doing any
		// serious kernel work.
		// LAB 4: Your code here.
		assert(curenv);

		lock_kernel();

		// Garbage collect if current enviroment is a zombie
		if (curenv->env_status == ENV_DYING) {
			env_free(curenv);
			curenv = NULL;
			sched_yield();
		}

		// Copy trap frame (which is currently on the stack)
		// into 'curenv->env_tf', so that running the environment
		// will restart at the trap point.
		curenv->env_tf = *tf;
		// The trapframe on the stack should be ignored from here on.
		tf = &curenv->env_tf;
	}

	// Record that tf is the last real trapframe so
	// print_trapframe can print some additional information.
	last_tf = tf;

	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);
	//cprintf("trap trap_dispatch returned\n");

	// If we made it to this point, then no other environment was
	// scheduled, so we should return to the current environment
	// if doing so makes sense.
	if (curenv && curenv->env_status == ENV_RUNNING)
		env_run(curenv);
	else
		sched_yield();
}


void _pgfault_upcall(void);

void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.

	// LAB 3: Your code here.

	//cprintf("page_fault_handler  envid %x, va %p, tf_eip %p, tf_esp %p\n", 
	//	curenv->env_id, fault_va, tf->tf_eip, tf->tf_esp);

	// in kernel mode, the 2 lowest bits of the faultee's CS register are 0
	if ((tf->tf_cs & 0x3) == 0) {	
		panic("page fault in kernel mode  va %08p", fault_va);
	}

	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Call the environment's page fault upcall, if one exists.  Set up a
	// page fault stack frame on the user exception stack (below
	// UXSTACKTOP), then branch to curenv->env_pgfault_upcall.
	//
	// The page fault upcall might cause another page fault, in which case
	// we branch to the page fault upcall recursively, pushing another
	// page fault stack frame on top of the user exception stack.
	//
	// The trap handler needs one word of scratch space at the top of the
	// trap-time stack in order to return.  In the non-recursive case, we
	// don't have to worry about this because the top of the regular user
	// stack is free.  In the recursive case, this means we have to leave
	// an extra word between the current top of the exception stack and
	// the new stack frame because the exception stack _is_ the trap-time
	// stack.
	//
	// If there's no page fault upcall, the environment didn't allocate a
	// page for its exception stack or can't write to it, or the exception
	// stack overflows, then destroy the environment that caused the fault.
	// Note that the grade script assumes you will first check for the page
	// fault upcall and print the "user fault va" message below if there is
	// none.  The remaining three checks can be combined into a single test.
	//
	// Hints:
	//   user_mem_assert() and env_run() are useful here.
	//   To change what the user environment runs, modify 'curenv->env_tf'
	//   (the 'tf' variable points at 'curenv->env_tf').

	// LAB 4: Your code here.

	if (curenv->env_pgfault_upcall == NULL) {

		switch(tf->tf_err) {
			case 4:
				cprintf("I read an unmapped virtual address from location %x!\n",
					 fault_va);
				break;
			case 5:
				cprintf("I read a protected virtual address from location %x!\n",
					 fault_va);
				break;
			case 6:
				break;
			case 7:
				break;
		}

	} else {
		//cprintf("page_fault_handler stack check  curenv %x from %p\n", 
				//curenv->env_id, (UXSTACKTOP - PGSIZE));

		user_mem_assert(curenv, (void*) (UXSTACKTOP - PGSIZE), PGSIZE, PTE_W);

		struct UTrapframe* utf;

		// To test whether tf->tf_esp is already on the user exception 
		// stack, check whether it is in the range between 
		// UXSTACKTOP-PGSIZE and UXSTACKTOP-1, inclusive. 	

		if (tf->tf_esp >= (UXSTACKTOP - PGSIZE) && 
		    tf->tf_esp < UXSTACKTOP) {

			// There was a recursive fault.
			// This means that the trap frame esp points to 
			// the user-exception-stack.

			// allocate user trap frame; remember extra word
			utf = (struct UTrapframe*)((char*) 
				tf->tf_esp - 4 - sizeof(struct UTrapframe));

			//cprintf("page_fault_handler tf_esp %p, utf %p, sz %d\n",
			//	tf->tf_esp, utf, 
			//	sizeof(struct UTrapframe) + sizeof(int));

			// check if there is user-exception-stack overflow
			user_mem_assert(curenv, (void*) utf, 1, PTE_W);

		} else {
			// Page fault happend when executing with 
			// regular user stack (normal user execution).

			// Allocate user trap frame at the top of the
			// user-exception-stack; there is no need to check
			// if this stack overflows, still plenty of room
			utf = (struct UTrapframe*) 
				(UXSTACKTOP - sizeof(struct UTrapframe));
		}

		// Write user-exception-stack frame.
		// The information in this frame will be used when a
		// user page fault handler has completed and switches back
		// the the user instruction that caused the page fault.
		// This switch happens in user mode, not from the kernel.
		
        	utf->utf_fault_va = tf->tf_trapno == T_PGFLT ? fault_va : 0;  
        	utf->utf_err      = tf->tf_err;
        	utf->utf_regs     = tf->tf_regs;
        	utf->utf_eip      = tf->tf_eip;
        	utf->utf_eflags   = tf->tf_eflags;
        	utf->utf_esp      = tf->tf_esp;

		// Modify the trap frame so that, when returning to
		// user mode, execution will continue at the user page fault
		// handler, with the user-exception-stack.
		//
		// After this happens, the information that the trap mechanism
		// saved in the trap frame, which is needed to return to
		// the user instruction that caused the page fault, will
		// have been popped from the kernel stack. This is the 
		// reason why it had to be saved in the user-exception-stack,
		// in the user trap frame.

        	tf->tf_eip = (uintptr_t) curenv->env_pgfault_upcall;
		tf->tf_esp = (uintptr_t) utf; 
		env_run(curenv);   // doesn't return
	}

	// Destroy the environment that caused the fault.
	cprintf("[%08x] user fault va %08x ip %08x\n",
		curenv->env_id, fault_va, tf->tf_eip);
	print_trapframe(tf);
	env_destroy(curenv);
}

