# Lab1 answers
These are my answers to the exercises and questions in [Lab1](https://pdos.csail.mit.edu/6.828/2011/labs/lab1/)


## Exercise 5

The first boot loader instruction that would break if the VMA was changed by passing, say, ```-Ttext 0xDDDD``` to the linker in ```boot/Makefrag```, instead of the correct ```0x7C00```, would be the very first boot loader instruction, given that the BIOS would still be jumping to ```0x7C00```.

This is because the change in ```Makefrag``` results in the LMA and the VMA being set to the same address ```0xDDDD```, so the boot loader will be loaded at the wrong physical address

I confirmed this by actually running the badly configured boot loader.


## Exercise 6

They are different, because when the BIOS enters the boot loader nothing has been yet loaded at ```0x100000```, but when the boot loader enters the kernel, it will have loaded the kernel starting at that address.

I confirmed this by actually setting break points where the BIOS enters the boot loader and where the boot loader enters the kernel.


## Exercise 7

Before ```movl %eax, %cr0```, which enables paging,``` 0x00100000``` and ```0xf0100000``` are different. After executing that instruction, with paging enabled, the have the same content, because they were mapped to the same physical address.

The first instruction that would fail if the mapping wasn't in place would be jmp:
```
        mov     $relocated, %eax
f0100028:       b8 2f 00 10 f0          mov    $0xf010002f,%eax
        jmp     *%eax
f010002d:       ff e0                   jmp    *%eax
```

because it is the first instruction that refers to an address that is expected to have been mapped.

It should jump to:

  ``` 0xf010002f <relocated>:    mov    $0x0,%ebp```

If ```movl %eax, %cr0``` is commented out in ```kern/entry.S```, it instead jumps to:

  ```0xf010002e <relocated+2>:    add    %al,(%eax)```


## Exercise 8

Edited ```lib/printfmt.c```:
```
                case 'o':
                // it is just like case 'x', replacing the base
                        num = getuint(&ap, lflag);
                        base = 8;
                        goto number;
```
Recompiling and running in qemu prints the correct result:

```6828 decimal is 15254 octal!```


### Answers to questions:

**1)** The exported kernel/console.c function that is used by kernel/cprintf.c is:
```
void
cputchar(int c)
```
This function is used by cprintf.c to print individual characters to the console.

**2)** The code snippet for console.c:
```
1      if (crt_pos >= CRT_SIZE) {
2              int i;
3              memcpy(crt_buf, crt_buf + CRT_COLS, (CRT_SIZE - CRT_COLS) * sizeof(uint16_t));
4              for (i = CRT_SIZE - CRT_COLS; i < CRT_SIZE; i++)
5                      crt_buf[i] = 0x0700 | ' ';
6              crt_pos -= CRT_COLS;
7      }
```
seems to be scrolling down the screen by one line; ```CRT_SIZE``` seems to be the size of the screen buffer, and ```crt_pos``` the buffer pointer; ```memcpy``` seems to be copying all lines in the buffer except the last (one line seems to be ```CRT_COLS``` wide), and then seems to be filling the new line at the "bottom" (starting at ```CRT_SIZE - CRT_COLS```) with spaces; is ends by decrementing the buffer pointer by one line.

**3)** The easiest way of executing the code snippet is to edit ```kernel/init.c```

In the first call to ```cprintf```:
- ```fmt``` points to ```0xf0101a32```, the value is string ```"x %d, y %x, z %d\n"```
- ```ap```  points to``` 0xf010ffd4```, the value is the first argument, ```1```

Each call to ```cons_putc```, ```va_arg``` and ```vcprint```, in order:
```
f010090a	vcprintf	fmt=0xf0101a32 ap=0xf010ffd4
cons_putc "x"
cons_putc " "
cons_putc "1"
cons_putc ","
cons_putc " "
cons_putc "y"
cons_putc " "
cons_putc "3"
cons_putc ","
cons_putc " "
cons_putc "z"
cons_putc " "
cons_putc "4"
cons_putc "\n"
```
**4)** The code snippet is:
```
    unsigned int i = 0x00646c72;
    cprintf("H%x Wo%s", 57616, &i);
```
The output is:	```He110 World```

This is how it works:

-``` vcprintf``` is called with ```fmt``` pointing to ```"H%x Wo%s"``` and with ```ap``` pointing to ```57616```, which is in the stack.
```
- cons_putc("H")
- cons_putc("e")
- va_arg(pointer to 57616 in the stack, int)
- cons_putc("1")
- cons_putc("1")
- cons_putc("0")
- cons_putc(" ")
- cons_putc("W")
- cons_putc("o")
- va_arg(pointer to 0x72 in the stack, char*)
- cons_putc("r") <- 0x72
- cons_putc("l") <- 0x6c
- cons_putc("d") <- 0x64
```
If x86 was big-endian, to get the same output ```i = 0x726c6400```
There would be no need to change ```57616```, because in this case there is no dependency on the order in which bytes are stored in memory.

**5)** For the code snippet:	```cprintf("x=%d y=%d", 3);```

```y``` will be printed as whatever is the integer value of the 4 bytes that happen to be in the stack before ```3```.

**6)** I don't see how it could work, somehow the C code would need to be able to locate the first argument, the ```fmt```, which would sit in the stack on the top of an unknown number of other arguments each of which is of unknown size.


**Exercise 9**

The kernel initializes its stack in the following line in kern/entry.S:
```
        # Set the stack pointer
        movl    $(bootstacktop),%esp
f0100034:       bc 00 00 11 f0          mov    $0xf0110000,%esp
```
The initial value of the stack pointer is ```0xf0110000```, which is address ```0x10000``` in physical memory.

The kernel "reserves" space for its stack by not allocating any memory below the stack pointers initial address; the kernel stack will go from ```0x10000``` down to ```0x0```

The stack pointer is initialized to the top of the stack, which grows down from there.


**Exercise 10**

The stack pointer is moved by 28 bytes (7 32 bit words) between entering ```test_bactrace``` and its recursive call:
```
0xf010ffcc	-1	push   %ebp		// saving ebp
0xf010ffc8	-1	push   %ebx		// saving ebx
0xf010ffc4	-3	sub    $0xc,%esp	// allocate 12 bytes (???)
0xf010ffb8	-1	push   %ebx		// cprintf call 2nd argument
0xf010ffb4	-1	push   $0xf0101980	// cprintf call 1st argument
			<call cprintf>
0xf010ffb0	+4	add    $0x10,%esp	// delete 2 args and 8 bytes
0xf010ffc0	-3	sub    $0xc,%esp	// allocate 12 bytes (???)
0xf010ffb4	-1	push   %eax		// push 1 arg
0xf010ffb0		<recursive call to test_backtrace>

0xf010ffcc - 0xf010ffb0 = 0x1c = 28 bytes = 7 byte words
```
Each recursive nesting level of ```test_backtrace``` pushes 7 4 byte words on the stack.

**Exercise 11**

See code for function ```mon_backtrace``` in ```kern/monitor.c```

**Exercise 12**

In ```obj/kern/kernel.sym```
```
...
f0102138 R __STAB_BEGIN__
f0105ab0 R __STAB_END__
f0105ab1 R __STABSTR_BEGIN__
f0107419 R __STABSTR_END__
...
```
In ```kern/kernel.ln```
```
...
        /* Include debugging information in kernel memory */
        .stab : {
                PROVIDE(__STAB_BEGIN__ = .);
                *(.stab);
                PROVIDE(__STAB_END__ = .);
                BYTE(0)         /* Force the linker to allocate space
                                   for this section */
        }

        .stabstr : {
                PROVIDE(__STABSTR_BEGIN__ = .);
                *(.stabstr);
                PROVIDE(__STABSTR_END__ = .);
                BYTE(0)         /* Force the linker to allocate space
                                   for this section */
        }
...
```
```objdump -h obj/kern/kernel```
```
...
  2 .stab         00003979  f0102138  00102138  00003138  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .stabstr      00001969  f0105ab1  00105ab1  00006ab1  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
...
```
```objdump -G obj/kern/kernel```
```
obj/kern/kernel:     file format elf32-i386

Contents of .stab section:

Symnum n_type n_othr n_desc n_value  n_strx String

-1     HdrSym 0      1225   00001968 1     
0      SO     0      0      f0100000 1      {standard input}
1      SOL    0      0      f010000c 18     kern/entry.S
2      SLINE  0      44     f010000c 0      
3      SLINE  0      56     f0100015 0      
4      SLINE  0      57     f010001a 0      
5      SLINE  0      59     f010001d 0      
6      SLINE  0      60     f0100020 0      
7      SLINE  0      61     f0100025 0      
8      SLINE  0      66     f0100028 0      
9      SLINE  0      67     f010002d 0      
10     SLINE  0      73     f010002f 0      
11     SLINE  0      76     f0100034 0      
12     SLINE  0      79     f0100039 0      
13     SLINE  0      82     f010003e 0      
14     SO     0      2      f0100040 31     kern/entrypgdir.c
15     OPT    0      0      00000000 49     gcc2_compiled.
16     LSYM   0      0      00000000 64     int:t(0,1)=r(0,1);-2147483648;2147483647;
17     LSYM   0      0      00000000 106    char:t(0,2)=r(0,2);0;127;
...
```
```init.s```
```
        .file   "init.c"
        .stabs  "kern/init.c",100,0,2,.Ltext0
        .text
.Ltext0:
        .stabs  "gcc2_compiled.",60,0,0,0
        .stabs  "int:t(0,1)=r(0,1);-2147483648;2147483647;",128,0,0,0
...
_tag:",128,0,0,0
        .stabn  162,0,0,0
        .stabn  162,0,0,0
        .stabs  "./inc/string.h",130,0,0,0
...
```

