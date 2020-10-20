# Homework 5: Locking

See [https://pdos.csail.mit.edu/6.828/2011/homework/xv6-lock.html](https://pdos.csail.mit.edu/6.828/2011/homework/xv6-lock.html)

The kernel panicked because the same CPU tried to reacquire the ide lock, which is a lock that it already held.

This happened because there was an interrupt, as can be seen from stack trace printed by ```panic```:
```
80104c06	call   8010055a <panic>		
801027a1	call   80104bdb <acquire>	
801064c8 	call   8010278e <ideintr>	
8010622b 	call   80106402 <trap>
...
```

Enabling interrupts while the file table lock is being held does not panic, because no interrupt will attempt to open a file.

```release()``` must clear ```lk->pcs[0]``` and ```lk->cpu``` *before* clearing ```lk->locked```.

Otherwise, the following race condition would be possible:

1.	cpu 1 clears ```lk->locked```
2.	cpu 2 acquires ```lk``` and sets ```lk->cpu``` to 2
3.	cpu 1 clears ```lk->cpu``` (sets it to 0)

If this happens, the ```lk```'s invariant broken, because now ```lk``` is locked by cpu 2, but ```lk->cpu``` is 0. This would prevent some bugs from panicking, for instance cpu 2 attempting to reacquire the lock.



