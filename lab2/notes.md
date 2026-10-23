# Laboratory Work No. 2 — Meet the OS You Will Build (xv6)

**Course:** Operating Systems, FCIM / FAF, UTM (2026–2027)  
**Student:** Botezatu Gabriela, FAF-243

## Part 1 — Building and booting xv6

The xv6 source was compiled and launched with QEMU:

```bash
cd ~/xv6-riscv
make qemu
```

The emulator displayed `xv6 kernel is booting` and `init: starting sh`, followed by the `$` prompt. QEMU can be exited with **Ctrl-A**, then **X**.

## Part 2 — Using xv6

Commands tested in the xv6 shell:

```sh
ls
cat README
echo hello xv6
ls | grep c
wc README
```

### Observe — answers

1. **Three programs included with xv6:** `ls`, `cat`, and `echo`. Other examples include `grep`, `wc`, and `mkdir`.
2. **Two OS features needed for a pipe:** Process management is needed to execute the two programs as separate processes; inter-process communication (IPC) is needed to transmit the first program's output to the second program's input through a kernel-managed pipe.
3. **xv6 shell compared with Linux shell:** The xv6 shell supports basic commands, arguments, pipes, and redirection, but is much simpler than Bash and lacks features such as command history, tab completion, shell variables, and advanced scripting.

**Observed output examples:** `echo hello xv6` printed `hello xv6`; `ls | grep c` displayed names containing `c`; `wc README` counted the lines, words and bytes in README.

## Part 3 — Reading the source code

Commands used in Ubuntu after leaving QEMU:

```bash
sed -n '1,40p' user/ls.c
sed -n '1,30p' user/cat.c
cat user/user.h
grep -n 'sys_read\|sys_write' kernel/sysfile.c | head
```

### Observe — answers

1. **System calls used by `user/cat.c`:** `open()` opens a file and returns a file descriptor; `read()` reads bytes into a buffer; `write()` sends bytes to an output descriptor; `close()` releases a file descriptor; and `exit()` terminates the program.
2. **Location of `sys_read`:** In the xv6 version examined for the report, `sys_read()` was found in `kernel/sysfile.c` at line **69** (`sys_write()` at line **83**). Line numbers can differ between source revisions.
3. **Difference between `kernel/` and `user/`:** `kernel/` contains privileged operating-system code and system-call implementations, whereas `user/` contains ordinary applications that request kernel services through system calls.

## Part 4 — Implementing and testing `sleep`

The program `user/sleep.c` was added to the xv6 source tree, and `$U/_sleep\` was included in the `UPROGS` section of the Makefile. In the xv6 version used for the report, the program invokes `pause(atoi(argv[1]))` because the delay interface is called `pause()` rather than `sleep()`.

```sh
# Run inside xv6 after rebuilding with make qemu:
sleep 10
sleep
```

**Observed results:** `sleep 10` paused briefly and returned to the shell prompt; calling `sleep` with no argument printed `usage: sleep <ticks>`.

## Conclusion

This laboratory introduced xv6 as a small Unix-like operating system that can be compiled, booted, inspected, and extended. By working with its shell and reading the source code, I understood how user programs request services from the kernel through system calls. Implementing and testing the `sleep` command demonstrated how a new user program is integrated into the xv6 build and executes inside the operating system.

*These notes summarize the experiments and observations documented in `Lab1-2_BotezatuGabriela_FAF243.docx`.*
