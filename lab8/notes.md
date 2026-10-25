# Laboratory Work No. 8 — The Same Calls, Inside Your Own OS

**Course:** Operating Systems (FCIM / FAF / UTM, 2026–2027)  
**Student:** Botezatu Gabriela, FAF-243  
**Environment:** Ubuntu 24.04 LTS, xv6-riscv in QEMU

This file contains the answers to all five **Observe** questions from Lab 8. The terminal screenshots and fuller explanations are included in `Lab8_BotezatuGabriela_FAF243.docx`.

## Part 1 — `run`, inside xv6

### Observe 1 — Which three calls are used in xv6 and on Linux? What is the same?

- **xv6 (Lab 8):** `fork()`, `exec("echo", args)`, `wait(&status)`.
- **Linux (Lab 7):** `fork()`, `execlp("ls", "ls", "-l", NULL)`, `waitpid(pid, &status, 0)`.

Both programs follow the same three steps: `fork` creates a child process, `exec` replaces the child's program, and the parent waits for the child to finish. A successful `exec` does not return in either system. Linux `execlp` searches `PATH`, whereas xv6 `exec` uses the supplied path and an argument array. In xv6, `wait` returns the child's exit status directly through its argument; on Linux, the status returned by `waitpid` is decoded with `WEXITSTATUS`.

**Commands tested:** `make qemu`, followed by `run` in the xv6 shell, after adding `$U/_run\` to `UPROGS`.

### Observe 2 — Who prints "hello from the child": `run` or `echo`?

The **`echo` program** prints `hello from the child`. After the child executes `exec("echo", args)`, the child process is replaced with `echo`, which prints the arguments it received. Its PID remains the same. The line `printf("exec failed\n")` after `exec` is not executed when `exec` succeeds.

**Observed output (echo version):**

```text
parent: my pid is 3
child: my pid is 4
hello from the child
parent: child 4 ended with status 0
```

### Observe 3 — Why is the exit status 1 in xv6 but 127 on Linux?

When `exec` fails because `nosuchprog` does not exist, execution continues with the lines after `exec`. The xv6 version explicitly calls `exit(1)`, while the Linux Lab 7 version called `exit(127)`. Therefore, the parent reports **1 in xv6** and **127 on Linux**. The number is chosen by the program, not automatically by `exec`. The Linux value 127 is also the usual shell convention for “command not found.”

**Observed output (failed exec version):**

```text
exec failed
parent: child 4 ended with status 1
```

The child PID can change between runs.

## Part 2 — `pingpong`

### Observe 1 — Why are TWO pipes needed instead of one for both directions?

Each pipe is a one-way communication channel. `p2c` carries the byte **from parent to child**, while `c2p` carries it **from child to parent**. If both processes read and write through the same pipe, the parent could read back the byte it sent before the child receives it. Two separate pipes avoid that problem and make the direction of communication clear. Each process also closes the pipe ends it does not use.

**Commands tested:** `make qemu`, followed by `pingpong` in the xv6 shell, after adding `$U/_pingpong\` to `UPROGS`.

**Observed output (first run):**

```text
4: received ping
3: received pong
```

### Observe 2 — Which call makes the parent wait for the child's reply?

The parent blocks at **`read(c2p[0], &b, 1)`**. Because the child-to-parent pipe initially contains no data, `read` waits until the child writes the byte with `write(c2p[1], &b, 1)`. This guarantees that `received ping` is printed before `received pong`. The final `wait(0)` collects the finished child; it is not the call that waits for the reply byte.

## Optional exercise — `primes`

The optional `primes` program was also completed and tested. It uses a chain of processes connected by pipes to print the prime numbers from 2 through 35:

```text
prime 2
prime 3
prime 5
prime 7
prime 11
prime 13
prime 17
prime 19
prime 23
prime 29
prime 31
```

Each stage filters out multiples of its prime number, closes unused pipe ends, and exits when its input reaches end of file.

## Conclusion

Lab 8 demonstrates that `fork`, `exec`, and `wait` follow the same process-creation pattern in Linux and xv6, despite differences in their interfaces. `pingpong` shows how two one-way pipes and a blocking `read` coordinate communication between parent and child processes.
