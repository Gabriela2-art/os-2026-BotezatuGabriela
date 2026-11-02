# Laboratory Work 9 — Watching System Calls with `strace`

**Course:** Operating Systems  
**Student:** Botezatu Gabriela  
**Group:** FAF-243  

## Objective

Observe how Linux programs interact with the kernel through system calls using `strace`. Inspect the calls made by `ls -l`, then implement and trace a C program that opens, writes, reads and closes a file.

## Part 1 — System calls made by `ls -l`

**Command:**

```bash
strace -f -e trace=openat,read,write,close ls -l
```

### Observe: Which system call does `ls` use to print to the screen? To which descriptor does it write?

`ls` uses the **`write()`** system call to print its output. It writes to file descriptor **1 (`stdout`)**, which normally refers to the terminal. In the trace, this appears as `write(1, ..., ...)`.

The trace also contains `openat`, `read` and `close` calls used to load shared libraries and access files needed by the command.

### Approximately how many calls does one `ls -l` make?

To count trace lines, run:

```bash
strace -f -e trace=openat,read,write,close ls -l 2>&1 >/dev/null | wc -l
strace ls -l 2>&1 >/dev/null | wc -l
```

**Recorded results:**
- Trace lines for the four selected kinds: **[enter the observed number]**.
- Trace lines for all system calls: **[enter the observed number]**.

These counts depend on the installed libraries, Ubuntu environment and directory contents. The trace-line count may also include an exit summary, so it is an approximation of system-call counts.

## Part 2 — A program using system calls directly

**File:** `syscalls.c`

**Build and run:**

```bash
gcc -Wall -Wextra -o syscalls syscalls.c
./syscalls
```

**Expected program output:**

```text
open returned descriptor 3
write sent 14 bytes
read got 14 bytes: hello, kernel
```

The program creates (or truncates) `note.txt`, writes the 14-byte string `hello, kernel\n`, closes the file, reopens it in read-only mode, reads its contents and closes it again.

**Trace command:**

```bash
strace -e trace=openat,write,read,close ./syscalls
```

**Relevant calls (other library setup calls may also appear):**

```text
openat(AT_FDCWD, "note.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644) = 3
write(3, "hello, kernel\n", 14) = 14
close(3) = 0
openat(AT_FDCWD, "note.txt", O_RDONLY) = 3
read(3, "hello, kernel\n", 63) = 14
close(3) = 0
```

### Observe: Why is the first descriptor 3 rather than 0, 1 or 2?

Descriptors **0, 1 and 2** are already used for **standard input (`stdin`)**, **standard output (`stdout`)** and **standard error (`stderr`)**, respectively. A new file is assigned the lowest available file descriptor, which is normally **3**. After `close(3)`, the value 3 becomes available and can be reused by the next `open()`.

To demonstrate this, run:

```bash
./syscalls <&-
```

Closing standard input before starting the program normally makes descriptor **0** available for the file opened by the program.

### Why does `strace` show `openat()` rather than `open()`?

On this Linux system, the C library's `open()` wrapper typically performs the `openat()` system call with `AT_FDCWD`, meaning the path is interpreted relative to the process's current working directory.

### Why are there extra `write(1, ...)` calls?

The program uses `printf()` to display its results. `printf()` is a C library function, not itself a system call. It eventually uses `write()` on descriptor 1 to display text on the terminal.

## Conclusion

In Parts 1 and 2, `strace` made the interaction between Linux programs and the kernel visible. The `ls -l` command prints through `write()` on file descriptor 1 (`stdout`). The C program created `note.txt`, wrote 14 bytes to it, then reopened and read those bytes back. Its trace showed `openat()` for the C library's `open()` calls, along with the corresponding `write()`, `read()` and `close()` calls. The first opened file normally received descriptor 3 because descriptors 0, 1 and 2 are already reserved for standard input, standard output and standard error.
