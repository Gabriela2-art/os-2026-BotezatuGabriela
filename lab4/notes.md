# Laboratory Work No. 4 — The xv6 Shell from the Outside
## notes.md — Comparison Table and “Observe” Answers

**Student:** Botezatu Gabriela  
**Group:** FAF-243  
**Subject:** Operating Systems

## Part 1 — bash versus the xv6 Shell

The commands were tested in bash (inside `~/shtest`, containing `README`) and in xv6.

### Comparison table

| Command | Feature tested | bash | xv6 |
|---|---|---|---|
| `echo hello > f ; cat < f` | Redirection | `hello` | `hello` — redirection works. |
| `ls \| grep READ` | Pipe | `README` | The `README` line of `ls` (name, type, inode, size) — pipes work. |
| `(echo a; echo b) \| wc` | Grouping, list | `2 2 4` | `2 2 4` — grouping and lists work. |
| `echo one ; echo two` | Sequence | `one`, `two` | `one`, `two` — sequences work. |
| `echo bg &` | Background | `[1] <PID>`, `bg`, then `[1]+ Done` | `bg` printed without a job number, sometimes after the next `$` prompt. |
| `ls *` | Globbing | `f README` | `ls: cannot open *` — no globbing. |
| `echo $HOME` | Variables | `/home/gabi` | `$HOME` — no variables. |
| `echo "a b"` | Quotes | `a b` | `"a b"` — quotes are printed rather than removed. |
| `mkdir d ; cd d ; ls ..` | PATH / `cd` | `d f README` | `exec cd failed`; `ls ..` lists the root directory `/`. |
| Up arrow, Tab, Ctrl+R | History, completion | Previous command, completion, history search | Not supported; the arrow key sends an escape sequence; Tab and Ctrl+R do nothing useful. |

In xv6, `cd` is recognized only when it is at the beginning of the command line. In `mkdir d ; cd d ; ls ..`, the shell tries to execute `cd` as a program, resulting in `exec cd failed`. If `cd d` is run separately, `ls` stops working because xv6 has no `PATH` search:

```text
$ mkdir d
$ cd d
$ ls
exec ls failed
$ /ls ..
$ cd /
```

### Observe — Part 1

**1. Which features are common to both shells, and which are bash-only?**

Both shells run programs with arguments and support `<`, `>`, `>>`, pipes `|`, sequences with `;`, background execution with `&`, and grouping with parentheses `( )`. Only bash supports variables (`$HOME`), quoting, globbing (`*`), `PATH` search, job control (`jobs`, `%1`), command history, Tab completion, Ctrl+R history search, `&&`, `||`, command substitution, and scripting.

**2. Why does `ls` stop working after `cd d` in xv6?**

The xv6 shell has no `PATH` variable or program search. It passes the typed name directly to `exec()`, which searches relative to the current directory. The xv6 programs are in `/`, so after entering `d`, there is no `d/ls`. Running `/ls` or `/ls ..` works because the absolute path is provided. Alternatively, use `cd /`.

**3. Which missing feature is most useful, and which is easiest to add?**

The most missed feature would be globbing, together with command history, because manually typing file names is slow. A `PATH`-like search would be easiest to add: if `exec(argv[0])` fails and the name contains no `/`, the shell could retry with `/` added before the program name, in the `EXEC` case of `runcmd()`.

## Part 2 — Who Expands `*`? A Program That Prints Its `argv`

The xv6 program `user/args.c` prints `argc` and each `argv[i]`. It was added to `UPROGS` as `$U/_args\`. The Linux version, `args_linux.c`, was compiled with:

```bash
gcc -o args args_linux.c
```

Tested commands:

```text
args one              two         three
args *.c "x y"
args $HOME
args 'single quoted'
```

### Comparison: `args *.c "x y"`

| xv6 | bash (Linux) |
|---|---|
| `argc = 4` | `argc = 3` |
| `argv[0] = [args]` | `argv[0] = [./args]` |
| `argv[1] = [*.c]` | `argv[1] = [args_linux.c]` |
| `argv[2] = ["x]` | `argv[2] = [x y]` |
| `argv[3] = [y"]` | — |

For `args $HOME`, xv6 receives `[$HOME]`, while bash receives `[/home/gabi]`. For `args 'single quoted'`, xv6 receives two arguments (`['single]` and `[quoted']`), while bash receives one (`[single quoted]`).

### Observe — Part 2

**1. How many arguments does the program receive in each shell?**

In xv6, it receives four arguments: the program name, the literal `*.c`, and the two parts `"x` and `y"` with quotation marks preserved. In bash, it receives three: `./args`, `args_linux.c` (the matching C file), and `x y` as one argument without quotes. If the Linux folder contains more `.c` files, bash passes each matching file as an additional argument.

**2. Who expands `*` and removes the quotes?**

The shell does this before executing the program. The `args` program only prints the arguments it receives. The kernel copies the provided `argv` array into the new process without performing globbing or quote removal. Bash expands `*.c` and removes quotation marks; xv6 does neither.

**3. What happens to the extra spaces in `args one              two         three`?**

In both shells, the extra spaces disappear. The output has `argc = 4`, with `[one]`, `[two]`, and `[three]` as the three arguments after the program name. Both shells split input on blanks, so the original number of spaces is not preserved.

## Part 3 — Reading the Shell

Source inspection commands from the report:

```bash
cd ~/xv6-riscv
wc -l user/sh.c
grep -n "#define" user/sh.c
grep -n "case " user/sh.c
grep -n "cd" user/sh.c | head
```

The `user/sh.c` file has **499 lines**. Lines 8–12 define the five command types, and line 14 defines `MAXARGS 10`. The cases in `runcmd()` are at lines 75, 83, 93, 101 and 125. The search for `cd` found the error message at line 170; `cd` is handled in `main()` around line 166.

### Observe — Part 3

**1. What are the five command types and what creates each?**

| Type | Meaning | Syntax/example |
|---|---|---|
| `EXEC` (1) | Run a program with arguments | `ls ..` |
| `REDIR` (2) | Redirect input or output | `<`, `>`, `>>` |
| `PIPE` (3) | Connect commands with a pipe | `\|` |
| `LIST` (4) | Execute a sequence | `;` |
| `BACK` (5) | Run a command in the background | `&` |

Parentheses `( )` group commands but are not a separate command type.

**2. Where is `cd` handled, and why must it run in the parent?**

`cd` is handled in `main()`, before `fork()`. If the command line starts with `cd `, the shell calls `chdir()` itself:

```c
if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == ' ') {
  // Chdir must be called by the parent, not the child.
  cmd[strlen(cmd) - 1] = 0; // chop \n
  if (chdir(cmd + 3) < 0)
    fprintf(2, "cannot cd %s\n", cmd + 3);
} else {
  if (fork1() == 0)
    runcmd(parsecmd(cmd));
  wait(0);
}
```

The working directory belongs to a process. If `cd` executed in a child, it would change only the child's directory; the shell would remain in its original directory. Since xv6 recognizes `cd` only at the start of the entire line, it fails when placed in the middle of a list.

**3. Where does xv6 handle `*` or `$`?**

It does not. In `sh.c`, `$` appears only as the shell prompt string `"$ "` (line 137), while `*` appears as a C pointer operator. There is no globbing or variable-expansion implementation. As observed, `ls *` treats `*` literally, `echo $HOME` prints `$HOME`, and quotation marks are not removed.
