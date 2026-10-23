# Laboratory Work No. 7 — Three Small Programs: Processes, Redirection, a Pipe

**Course:** Operating Systems (FCIM / FAF / UTM, 2026–2027)  
**Student:** Botezatu Gabriela, FAF-243  
**Environment:** Ubuntu 24.04 LTS, Bash, GCC  

This file contains the answers to the **Observe** questions and the supporting commands and results documented in `Lab7_BotezatuGabriela_FAF243(2).docx`.

## Part 1 — Creating a Process with `fork`, `exec` and `wait`

The program `run.c` creates a child with `fork()`. In the child, `execlp()` replaces the program with `ls -l`; the parent calls `waitpid()` and prints its exit status.

```bash
gcc -Wall -Wextra -o run run.c && ./run
```

**Output recorded for `ls -l` (selected lines):**

```text
parent: my PID is 5179
child: my PID is 5180, my parent is 5179
total 24
... output of ls -l ...
parent: child 5180 ended with status 0
```

The `execlp` call was then changed to `execlp("date", "date", (char *)NULL);` and later to `execlp("nosuchprog", "nosuchprog", (char *)NULL);`. The commands were compiled and executed again after each change.

**Output recorded for `date`:**

```text
parent: my PID is 5457
child: my PID is 5458, my parent is 5457
Mon Oct 19 12:19:05 PM UTC 2026
parent: child 5458 ended with status 0
```

**Output recorded for `nosuchprog`:**

```text
parent: my PID is 5627
child: my PID is 5628, my parent is 5627
exec: No such file or directory
parent: child 5628 ended with status 127
```

### Observe — answers

1. **Why is `perror("exec")` printed only for `nosuchprog`?** When `execlp()` succeeds, the child is replaced by the requested program (`ls` or `date`), so execution never returns to the code after `execlp()`. When the program does not exist, `execlp()` fails and returns `-1` with `errno = ENOENT`; the following `perror("exec")` therefore prints `exec: No such file or directory`.
2. **What is the number printed as “my parent”?** `getppid()` in the child returns the PID printed by the original parent process. For example, `5180` reported its parent as `5179`. The child PID also matches the number printed by the parent in the final `child ... ended` line, because `fork()` returns that PID to the parent.
3. **What exit status does the parent see?** With `ls` or `date`, the parent sees status **0**, indicating successful execution. With `nosuchprog`, it sees **127**, because the child calls `exit(127)` after `execlp()` fails. `waitpid()` collects the status, and `WEXITSTATUS()` extracts the exit code.

## Part 2 — A Zombie, on Purpose

In `zombie.c`, `sleep(20);` was added before `waitpid(pid, &status, 0);`, delaying the parent's collection of its terminated child.

```bash
gcc -Wall -Wextra -o zombie zombie.c
./zombie > /dev/null &
sleep 2; ps -o pid,ppid,stat,cmd
# After the parent has completed:
ps -o pid,ppid,stat,cmd
```

**Output shown in the report while the parent slept (selected lines):**

```text
PID   PPID STAT CMD
5997  4862 S    ./zombie
5999  5997 Z    [zombie] <defunct>
```

After the parent called `waitpid()`, neither the parent process nor its defunct child remained in the listing.

### Observe — answers

1. **What does `Z` in the `STAT` column mean?** `Z` means **zombie**: the child has terminated, but the parent has not yet called `waitpid()` to collect its exit status. The kernel keeps a process-table entry with its PID and status until it is reaped. The child's PPID identifies the sleeping parent.
2. **What happens after 20 seconds?** The parent finishes `sleep(20)`, calls `waitpid(pid, &status, 0)`, and reaps the child. The `<defunct>` entry disappears, and the parent terminates. The relevant line is `waitpid(pid, &status, 0);`.

## Part 3 — Redirection of Standard Output

`redirect.c` does the equivalent of `ls -l > out.txt` by closing file descriptor **1** in the child and opening `out.txt` before `execlp("ls", ...)`.

```bash
gcc -Wall -Wextra -o redirect redirect.c && ./redirect
head -3 out.txt
```

**Output recorded:**

```text
open gave descriptor 1
done: the listing is in out.txt
```

The listing itself was stored in `out.txt`. The report's `head -3 out.txt` output starts with `total 68`, followed by directory-listing lines, including `out.txt` with a size of 0 at the moment it was listed.

### Observe — answers

1. **Which descriptor number did `open()` return?** It returned **1**. Since descriptor 1 had just been closed and descriptors 0 and 2 remained open, it was the lowest available descriptor number and was reused for `out.txt`.
2. **Why is the listing in the file instead of on the screen?** `ls` continues writing to standard output (descriptor 1), but that descriptor now refers to `out.txt`. The descriptor table survives `exec`. The diagnostic `open gave descriptor 1` stays on the screen because it is written to standard error (descriptor 2); the parent's `done` message stays on the screen because only the child redirected its output.
3. **How could standard error be redirected instead?** Close descriptor **2** in the child and open `err.txt` in its place. `open()` would then obtain descriptor 2, so subsequent error messages would be written to the file, equivalent to `2> err.txt`, while standard output would remain on the terminal.

## Part 4 — Building a Pipe by Hand

`pipeline.c` creates a pipe with `p[0]` as the reading end and `p[1]` as the writing end. The first child runs `ls /etc`, writing to the pipe; the second runs `wc -l`, reading from it. The parent closes both ends and waits for both children.

| Process | Descriptor 0 (stdin) | Descriptor 1 (stdout) | Original pipe ends |
|---|---|---|---|
| Child 1: `ls` | Terminal | Pipe writing end | Closed after `dup()` |
| Child 2: `wc -l` | Pipe reading end | Terminal | Closed after `dup()` |
| Parent | Terminal | Terminal | Both closed |

```bash
gcc -Wall -Wextra -o pipeline pipeline.c && ./pipeline
ls /etc | wc -l
```

**Output recorded:**

```text
236
236
```

Then the parent-side `close(p[1]);` was commented out and the program was compiled and run again. The program did not finish and was interrupted with **Ctrl+C**.

### Observe — answers

1. **Do the program and shell pipeline print the same result?** Yes. Both printed **236** in the report. The program manually performs the same descriptor connections as Bash for `ls /etc | wc -l`: `ls` writes to descriptor 1, and `wc` reads from descriptor 0.
2. **Which child never finishes when the parent does not close `p[1]`?** The `wc -l` child remains blocked, waiting for the input to end. The `ls` child finishes, but the parent waits for `wc`, so the program does not terminate.
3. **What exactly is `wc` waiting for?** It is waiting for **EOF** on the pipe. A pipe's reader receives EOF only after **all** open writing ends are closed. Because the parent kept `p[1]` open, `wc` cannot receive EOF even though `ls` has finished. The parent is simultaneously waiting for `wc`.

## Optional Task — Three-Stage Pipeline

`pipeline3.c` implements `ls /usr/bin | grep x | wc -l` with **two pipes and three children**. Each process closes all unused pipe ends after connecting the required descriptors.

```bash
gcc -Wall -Wextra -o pipeline3 pipeline3.c && ./pipeline3
ls /usr/bin | grep x | wc -l
```

**Output recorded:**

```text
224
224
```

Both commands print the same result. When `ls` exits, `grep` receives EOF on the first pipe; after `grep` exits, `wc` receives EOF on the second pipe and prints the count. Closing unused descriptors prevents the hanging behaviour observed in Part 4.

## Conclusion

The experiments demonstrate how `fork()`, `exec()`, and `waitpid()` create, replace, and reap processes; how file descriptors make redirection possible; and how pipes connect separate programs. The observations also show why a failed `exec` returns to the caller, why a child becomes a zombie before being reaped, and why every unused pipe-writing end must be closed to allow readers to receive EOF.
