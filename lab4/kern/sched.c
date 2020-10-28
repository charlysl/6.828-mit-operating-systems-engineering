#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>


// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;
	int i;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING) and never choose an
	// idle environment (env_type == ENV_TYPE_IDLE).  If there are
	// no runnable environments, simply drop through to the code
	// below to switch to this CPU's idle environment.

	// LAB 4: Your code here.

	int last_id = 0;
	if (curenv != NULL) {
		last_id = ENVX(curenv->env_id);
	}

	int id;
	struct Env* e = NULL;
	for (id = (last_id + 1) % NENV; id != last_id; id = (id + 1) % NENV) {
		e = &envs[id];
		if (e->env_type != ENV_TYPE_IDLE && e->env_status == ENV_RUNNABLE) {
			break;
		} else {
			e = NULL;
		}
	}

	if (id == last_id && envs[last_id].env_status == ENV_RUNNABLE) {
		e = curenv;
	}

	if (e != NULL) {
		//cprintf("sched_yield run  id %d, last_id %d, cpu %d\n", 
	        //			id, last_id, cpunum());
		env_run(e);
		panic("returned to sched_yield");
	}

	// For debugging and testing purposes, if there are no
	// runnable environments other than the idle environments,
	// drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if (envs[i].env_type != ENV_TYPE_IDLE &&
		    (envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING))
			break;
	}
	if (i == NENV) {
		cprintf("No more runnable environments!\n");
		while (1)
			monitor(NULL);
	}

	// Run this CPU's idle environment when nothing else is runnable.
	idle = &envs[cpunum()];
	if (!(idle->env_status == ENV_RUNNABLE || idle->env_status == ENV_RUNNING))
		panic("CPU %d: No idle environment!", cpunum());
	env_run(idle);
}
