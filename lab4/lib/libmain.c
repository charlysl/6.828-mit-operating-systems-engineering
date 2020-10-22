// Called from entry.S to get us going.
// entry.S already took care of defining envs, pages, vpd, and vpt.

#include <inc/lib.h>
#include <inc/env.h>

extern void umain(int argc, char **argv);

const volatile struct Env *thisenv;
const char *binaryname = "<unknown>";

void
libmain(int argc, char **argv)
{
	// set thisenv to point at our Env structure in envs[].
	// LAB 3: Your code here.

	envid_t envid = sys_getenvid();

	thisenv = &envs[ENVX(envid)];

	cprintf("libmain  thisenv %08p, thisenv->env_id %d, sys_getenvid 0x%08x, ENVX %d, envs 0x%08p, sizeof(Env) %d\n", thisenv, thisenv->env_id, envid, ENVX(envid), envs, sizeof(struct Env));

	// save the name of the program so that panic() can use it
	if (argc > 0)
		binaryname = argv[0];

	// call user main routine
	umain(argc, argv);

	// exit gracefully
	exit();
}

