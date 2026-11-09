# Laboratory Work No. 10 — Add Your Own System Call: `trace`

**Course:** Operating Systems (FCIM / FAF / UTM, 2026–2027)  
**Student:** Botezatu Gabriela, FAF-243  
**Environment:** Ubuntu 24.04 LTS, xv6-riscv in QEMU

This file contains the answers to all four **Observe** questions from Part 3 of Lab 10. The terminal screenshots and fuller explanations are included in `Lab10_BotezatuGabriela_FAF243.docx`.

## Part 3 — Play with the Mask

### Observe 1 — In `trace 32 grep hello README`, why do you see only `read`, and not `open` or `close`?

The trace mask selects which system calls the kernel prints. **32 = 2⁵**, so only bit 5 is set, and system call number 5 is **`read`**. The kernel checks `(p->trace_mask >> num) & 1` before printing each call. The bits for `open` (15) and `close` (21) are not set, so those calls still happen but are not displayed.

**Command tested:** `trace 32 grep hello README`

**Observed output:**

```text
3: syscall read -> 1023
3: syscall read -> 965
3: syscall read -> 453
3: syscall read -> 0
```

The PID may differ between runs.

### Observe 2 — In `forkforkfork`, three different PIDs print lines. Which line of code from Part 2 makes that possible?

The line **`np->trace_mask = p->trace_mask;`** in `fork()` in `kernel/proc.c` copies the parent's trace mask to each child. As a result, child processes also trace their own `fork` calls. Without this line, the children would not inherit the mask, so only the original traced process would print its calls. The tested run showed lines from processes with PIDs 7, 8, and 9.

**Command tested:** `trace 2 usertests forkforkfork`

**Relevant code:**

```c
np->trace_mask = p->trace_mask;
```

**Observed result:** Several processes printed `syscall fork -> ...` lines. When the process table became full, some `fork` calls returned `-1`. The test finished with `OK` and `ALL TESTS PASSED`.

### Observe 3 — In `trace 65536 echo hi`, why is the trace line printed between `hi` and the newline? How many times does `echo` call `write`?

**`echo` calls `write` twice**: first to print `hi` (2 bytes), and then to print the newline character (1 byte). The kernel prints the trace message immediately after each `write` system call returns. Therefore, the first trace message appears after `hi` but before the second `write` prints the newline.

**Command tested:** `trace 65536 echo hi`

**Observed output:**

```text
hi67: syscall write -> 2

67: syscall write -> 1
```

The PID may differ between runs.

### Observe 4 — Which mask traces both `open` (15) and `close` (21), and nothing else?

Each system call is selected by setting the bit corresponding to its number. To trace only `open` and `close`, combine **bit 15 and bit 21**:

```text
(1 << 15) | (1 << 21)
= 32768 + 2097152
= 2129920
```

The required mask is **`2129920`** (hexadecimal `0x208000`). It selects `open` and `close` without selecting `read` or other system calls.

**Command tested:** `trace 2129920 grep hello README`

**Observed output:**

```text
68: syscall open -> 3
68: syscall close -> 0
```

The PID may differ between runs.

## Conclusion

Lab 10 demonstrates how a trace mask selects system calls by their numbers. The tests show that `read`, `fork`, and `write` can be traced individually; child processes inherit the mask through `fork`; and bits can be combined to trace exactly `open` and `close`. The kernel prints each selected system call after it returns.
