# Laboratory Work No. 2 – Meet the OS You Will Build (xv6)

**Course:** Operating Systems  
**Student:** Botezatu Gabriela  
**Group:** FAF-243  
**University:** Technical University of Moldova  
**Academic year:** 2026–2027

## About this file

This file contains the short answers to the **Observe** questions from **Part 2 – Use xv6 as the Unix it is** and **Part 3 – Read the source**, as required for Laboratory Work No. 2. The answers are based on the experiments and source-code inspection documented in the laboratory report.

---

## Part 2 – Use xv6 as the Unix it is

### Observe

**1. List three programs xv6 ships with.**  
Three programs included in xv6 are `ls`, `cat`, and `echo`.

**2. Which two OS features must exist for a pipe (`|`) to work?**  
A pipe requires **process management** to run separate processes and **inter-process communication (IPC)** to transfer the output of one process to the input of another.

**3. How does the xv6 shell compare to the Linux shell from Lab 1?**  
The xv6 shell supports basic Unix commands, arguments, pipes, and redirection, but it is simpler than the Linux shell and lacks command history, tab completion, variables, and advanced scripting.

---

## Part 3 – Read the source

### Observe

**1. Which system calls does `user/cat.c` use, and what does each one ask the kernel to do?**  
`open()` opens a file and returns a file descriptor; `read()` reads data into a buffer; `write()` sends data to an output descriptor; `close()` releases the descriptor; and `exit()` terminates the program.

**2. In which kernel file, and on which line, is `sys_read` implemented?**  
In the xv6 version examined during the laboratory, `sys_read()` is implemented in `kernel/sysfile.c`, starting at **line 69**.

**3. What is the difference between the code in `kernel/` and the code in `user/`?**  
`kernel/` contains privileged operating-system code and system-call handlers, while `user/` contains ordinary programs that request kernel services through system calls.
