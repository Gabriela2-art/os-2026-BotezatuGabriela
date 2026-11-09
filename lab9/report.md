# Laboratory Work 9 — Watching System Calls with `strace`

**Course:** Operating Systems  
**Student:** Botezatu Gabriela  
**Group:** FAF-243  

## Objective

Observe how Linux programs interact with the kernel through system calls using `strace`. Inspect the calls made by `ls -l`, then implement and trace a C program that opens, writes, reads and closes a file. Finally, use `strace -c` to count system calls and compare output to the terminal with output redirected to a file.

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
- Trace lines for the four selected kinds: **approximately 40**.
- Trace lines for all system calls: **approximately 120** (in the earlier, nearly empty directory).

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

## Part 3 — Counting one kind of system call

The `-c` option makes `strace` display a summary table with the number of calls, errors and time spent in each system call, instead of printing each call individually. The `-e trace=write` option limits the summary to `write()` calls.

### 3.1 — Count all system calls made by `ls -l`

**Command:**

```bash
strace -c ls -l
```

**Recorded results:** The `ls -l` command in the `~/lab9` directory made **271 system calls of 35 kinds**, including **9 reported errors**. The most frequent calls included `getdents64` and `close` (34 each), `openat` (31), `fstat` (29), `mmap` (27) and `read` (20).

The total was higher than in Part 1 because the directory contained more files during this measurement. Many calls are for loading libraries and collecting file metadata rather than simply printing the listing. The reported errors did not prevent `ls` from finishing successfully.

### 3.2 — Count `write()` calls from `syscalls.c`

**Commands:**

```bash
strace -c -e trace=write ./syscalls
strace -c -e trace=write ./syscalls > out.txt
```

**Recorded results:**

| Output destination | `write()` calls |
| --- | ---: |
| Terminal | **4** |
| File (`out.txt`) | **2** |

When output goes to the terminal, the program makes one `write()` to `note.txt` and three separate writes for the `printf()` lines. When output is redirected, those three `printf()` lines are buffered together and written to `out.txt` in one call; the write to `note.txt` remains unchanged.

### 3.3 — Count `write()` calls from `ls -l /usr/bin`

**Commands:**

```bash
strace -c -e trace=write ls -l /usr/bin
strace -c -e trace=write ls -l /usr/bin > list.txt
```

**Recorded results:**

| Output destination | `write()` calls |
| --- | ---: |
| Terminal | **14** |
| File (`list.txt`) | **13** |

In this Ubuntu environment, `ls` already writes its large directory listing in blocks rather than printing each line through a separate system call. Redirecting its output therefore reduced the number of writes by only one.

### Observe — Why does redirecting output change the number of `write()` calls?

**Output buffering** can combine multiple pieces of text into one system call. For `syscalls.c`, the C library typically uses line buffering when `stdout` points to a terminal, but full buffering when it points to a file. Therefore, three `printf()` lines produce three terminal writes but only one redirected write, reducing the total from **4 to 2**. For the tested `ls -l /usr/bin`, the output was already grouped into large blocks, so the number changed only from **14 to 13**.

These are the results recorded in the Lab 9 report; counts can vary with the operating system, library implementation and directory contents.

## Conclusion

In Part 1, `strace` showed how `ls -l` interacts with the Linux kernel and prints its output using `write()` on file descriptor **1 (`stdout`)**. Even a simple directory listing requires many system calls for libraries, file metadata and other setup tasks.

In Part 2, `syscalls.c` created `note.txt`, wrote **14 bytes**, reopened the file and read those bytes back. Its trace showed `openat()`, `write()`, `read()` and `close()` in the expected order. The first file received descriptor **3** because descriptors 0, 1 and 2 were already used for standard input, output and error.

In Part 3, `strace -c` counted **271 calls of 35 kinds** for `ls -l` in the tested directory. Redirecting the output of `syscalls.c` reduced the number of `write()` calls from **4 to 2**, while `ls -l /usr/bin` changed from **14 to 13**. These measurements demonstrate how buffering and the output destination affect the number of system calls, even when the program itself does not change.
