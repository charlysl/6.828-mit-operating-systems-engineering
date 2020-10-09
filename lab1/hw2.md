# Homework 2

This are the answers to the questions in [Homework2](https://pdos.csail.mit.edu/6.828/2011/homework/xv6-intro.html)


This is the stack dump on entry in the kernel:
```
(gdb) x/24x $esp
0x7bdc:	0x00007db4	0x00000000	0x00000000	0x00000000
0x7bec:	0x00000000	0x00000000	0x00000000	0x00000000
0x7bfc:	0x00007c4d	0x8ec031fa	0x8ec08ed8	0xa864e4d0
0x7c0c:	0xb0fa7502	0xe464e6d1	0x7502a864	0xe6dfb0fa
0x7c1c:	0x16010f60	0x200f7c78	0xc88366c0	0xc0220f01
0x7c2c:	0x087c31ea	0x10b86600	0x8ed88e00	0x66d08ec0
```

The top of the stack is at ```7c00``` (not included), so any "stack" values from this address upwards are not in the stack.

The non-zero actual values in the stack (```0x00007db4``` and ```0x00007c4d```) are surely return address for function calls made in the boot loader, which was loaded at ```7c00```.

This can be confirmed by looking at ```bootblock.asm```:
```
...
  call    bootmain
    7c48:       e8 ee 00 00 00          call   7d3b <bootmain>

  # If bootmain returns (it shouldn't), trigger a Bochs
  # breakpoint if running under Bochs, then loop.
  movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
    7c4d:       66 b8 00 8a             mov    $0x8a00,%ax
...
```
```7c4d```, which must have been pushed first into the stack, is the address of the instruction right after the call to bootmain
```
...
  // Call the entry point from the ELF header.
  // Does not return!
  entry = (void(*)(void))(elf->entry);
  entry();
    7dae:       ff 15 18 00 01 00       call   *0x10018
}
    7db4:       8d 65 f4                lea    -0xc(%ebp),%esp
...
```
And ```7db4``` is the address of the instruction after the call to ```entry()```, which is how the kernel is entered.

