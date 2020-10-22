# Homework 7: sleep and wakeup

See (https://pdos.csail.mit.edu/6.828/2011/homework/xv6-sleep.html)[https://pdos.csail.mit.edu/6.828/2011/homework/xv6-sleep.html]

*One problem that the sleep() and wakeup() implementations avoid is races in which process A has decided to sleep but has not quite gone to sleep, at which point process B calls wakeup() but doesn't see that A is sleeping and thus does not wake A up. If it were possible for this to occur, A would have missed the event it was sleep()ing for, and its sleep() might never return.*


*__1__. How does the ```ptable.lock``` help avoid this problem? Give an ordering of events that could result in a wakeup being missed if the ```ptable.lock``` were not used. You need only include the relevant lines of code.*
```
wait(2389): sleep(proc)			// in proc->parent; proc not ZOMBIE yet
exit(2326): wakeup1(proc->parent)	// proc->parent not SLEEPING yet; missed
exit(2338): proc->state = ZOMBIE	// proc->parent never wakes; proc stays ZOMBIE for ever
```
The above sequence of events would have been avoided if ```proc->parent``` had acquired ```ptable.lock``` in ```wait()```, because then ```proc``` would have spinned when trying to acquire the same lock, and would have woken up ```proc->parent``` after finally acquiring the lock, which means that ```proc->parent```, who was holding the lock, would already have set its state to SLEEPING.

*__2__. ```sleep``` is also protected by a second lock, its second argument, which need not be the ```ptable.lock```. Look at the example in ```ide.c```, which uses the ```idelock```. Give an ordering of instructions that could result in a ```wakeup``` being missed if the ```idelock``` were not being used. (Hint: this should not be the same as your answer to question 1. The two locks serve different purposes.)*
```
iderw(3830): sleep(b)	// disk reading proc still RUNNING
ideint(3772): wakeup(b)	// disk reading proc missed
iderw(3830): sleep(b)	// disk reading proc set to SLEEPING
```
The problem is that there is a race condition, where a disk reading process decides to sleep in order to wait for the data to be available, but before it can change its state to ```SLEEPING```, the disk divice issues an interrupt to notify that the data is available; the interrupt handler checks of there is any process ```SLEEPING``` on the buffer, and because the reading process is not yet ```SLEEPING```, this is missed. When the reading process finally sets its state to ```SLEEPING```, it will never be woken.

This is avoided by using ```idelock```.

