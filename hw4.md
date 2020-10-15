# Homework: xv6	and Interrupts and Exceptions

See [https://pdos.csail.mit.edu/6.828/2011/homework/x86-intr.html](https://pdos.csail.mit.edu/6.828/2011/homework/x86-intr.html)

gdb's stack dump right after entering the first system call (breakpoint at ```0x80105205```) is:

```
(gdb) x/37x $esp
0x8dffff7c:	0x80106421	0x801030c8	0x00000400	0x000003d9
0x8dffff8c:	0x000000c8	0x0000000a	0x8010ff20	0x01010101
0x8dffff9c:	0x00000000	0x00000000	0x00000000	0x00000000
0x8dffffac:	0x80106213	0x8dffffb4	0x00000000	0x00000000
0x8dffffbc:	0x00000000	0x8dffffd4	0x00000000	0x00000000
0x8dffffcc:	0x00000000	0x00000007	0x00000000	0x00000000
0x8dffffdc:	0x0000002b	0x0000002b	0x00000040	0x00000000
0x8dffffec:	0x00000013	0x00000023	0x00000202	0x00000ff4
0x8dfffffc:	0x0000002b	Cannot access memory at address 0x8e000000
```

The meaning of each value is as follows:


```0x8dfffffc:	0x0000002b```	ss
```0x8dfffff8:	0x00000ff4```	esp
```0x8dfffff4:	0x00000202```	eflags
```0x8dfffff0:	0x00000023```	cs
```0x8dffffec:	0x00000013```	eip
```0x8dffffe8:	0x00000000```	error code
```0x8dffffe4:	0x00000040```	trapno
```0x8dffffe0:	0x0000002b```	%ds
```0x8dffffdc:	0x0000002b```	%es
```0x8dffffd8:	0x00000000```	%fs
```0x8dffffd4:	0x00000000```	%gs
```0x8dffffd0:	0x00000007```	%eax
```0x8dffffcc:	0x00000000```	%ecx
```0x8dffffc8:	0x00000000```	%edx
```0x8dffffc4:	0x00000000```	%ebx
```0x8dffffc0:	0x8dffffd4```	%oesp
```0x8dffffbc:	0x00000000```	%ebp
```0x8dffffb8:	0x00000000```	%esi
```0x8dffffb4:	0x00000000```	%edi	
```0x8dffffb0:	0x8dffffb4```	%esp, the trap frame, the arg for trap()
```0x8dffffac:	0x80106213```	return addr to alltraps, after call to trap()
```0x8dffffa8:	0x00000000```	%ebp
```0x8dffffa4:	0x00000000```	%edi
```0x8dffffa0:	0x00000000```	%esi
```0x8dffff9c:	0x00000000```	%ebx
```0x8dffff98:	0x01010101```	-1/7 ```sub    $0x1c,%esp``` (7 stack entries)
```0x8dffff94:	0x8010ff20```	-2/7	for local var (?)
```0x8dffff90:	0x0000000a```	-3/7	for local var (?)
```0x8dffff8c:	0x000000c8```	-4/7	for local var (?)
```0x8dffff88:	0x000003d9```	-5/7	for local var (?)
```0x8dffff84:	0x00000400```	-6/7	for local var (?)
```0x8dffff80:	0x801030c8```	-7/7	for local var (?)
```0x8dffff7c:	0x80106421```	return addr to trap(), after call to syscall()


```awk '{print "```"$1"\t"$2"```\n```\t\t"$3"```\n```\t\t"$4"```\n```\t\t"$5"```"}' t | tac```
