# Laboratory Work No. 3 — The Command Line: Shell, Pipes and Text Filters

**Student:** Botezatu Gabriela  
**Group:** FAF-243  
**Environment:** Ubuntu 24.04 LTS, Bash  
**Working directory:** `~/os-lab3`

This file records the predictions, outputs and answers for Parts 1–3, followed by the answers to Q1–Q8, as documented in the laboratory report.

## Part 1 — Moving Fast and Finding Help

### Commands and results

- Tab completion: `cd /et` was completed to `cd /etc/` when there was a unique match. With ambiguous input, Bash listed possible completions. `Ctrl+R` searched command history backwards and allowed an earlier command to be run again.
- Commands used to inspect the shell and executable search path:

```bash
type cd ; type ls ; type echo ; which ls
echo $PATH
echo $PATH | tr ':' '\n'
nosuchprog ; echo "exit code: $?"
```

**Observed results:** `cd` and `echo` were Bash built-ins, `ls` was an external program found at `/usr/bin/ls`. `PATH` contained **10 entries (9 unique directories)** because `/snap/bin` appeared twice. Running `nosuchprog` printed `command not found` and `exit code: 127`.

**Help commands:** `man ls` (search `/sort`, then `n`), `ls --help | head`, `man -k "list directory"`, and `help cd`.

### Observe — answers

1. **Why must `cd` be built into the shell?** A working directory belongs to a process. An external `cd` would only change its own child process's directory and could not change the parent shell's working directory.
2. **What does `PATH` contain?** In this environment it had 10 entries, 9 distinct: `/usr/local/sbin`, `/usr/local/bin`, `/usr/sbin`, `/usr/bin`, `/sbin`, `/bin`, `/usr/games`, `/usr/local/games` and `/snap/bin` (listed twice). Bash searches these directories for external commands; `ls` was found at `/usr/bin/ls`.
3. **What happens when a command is not found?** Bash prints `command not found` and sets the exit status to **127**.
4. **How does `ls -t` sort?** By modification time, newest first.

## Part 2 — What the Shell Does to the Command Line

### Setup

```bash
mkdir -p ~/os-lab3/play && cd ~/os-lab3/play
touch a.c b.c file1.txt file2.txt notes.txt "alfa beta.txt"
ls
```

### Predictions versus real outputs

| Command | My prediction | Real output | Explanation |
|---|---|---|---|
| `echo *.c` | `a.c b.c` | `a.c b.c` | Matched both `.c` files. |
| `echo file?.txt` | `file1.txt file2.txt` | `file1.txt file2.txt` | `?` matches one character. |
| `echo [a-b]*` | `a.c b.c` | `a.c alfa beta.txt b.c` | The pattern also matches a filename starting with `a` that contains a space. |
| `echo *.xyz` | Empty line | `*.xyz` | In this Bash setup, an unmatched glob remains unchanged. |
| `ls alfa beta.txt` | `alfa beta.txt` | Errors: `alfa` and `beta.txt` not found | An unquoted space separates arguments. |
| `ls "alfa beta.txt"` | `alfa beta.txt` | `alfa beta.txt` | Quoting preserves the filename as one argument. |
| `echo '$HOME' "$HOME" \$HOME` | `$HOME /home/gabi $HOME` | `$HOME /home/gabi $HOME` | Single quotes and the backslash protect `$`; double quotes allow expansion. |
| `echo {1..5} ; echo $((7 * 6))` | `1 2 3 4 5`, then `42` | `1 2 3 4 5`, then `42` | Brace and arithmetic expansion. |
| `echo "today is $(date +%A), I am $(whoami)"` | `today is Monday, I am gabi` | `today is Monday, I am gabi` | Command substitution. |
| `touch log{1..3}.txt && ls log*` | `log1.txt log2.txt log3.txt` | `log1.txt log2.txt log3.txt` | Brace expansion creates three files. |

### Observe — answers

1. **Which predictions were different?** The `[a-b]*` pattern also matched `alfa beta.txt`, and `*.xyz` remained literal rather than producing an empty line. The unquoted filename `alfa beta.txt` also caused `ls` to receive two arguments and fail.
2. **Who produces the filenames for `echo *.c`?** The **shell**, not `echo`. Bash expands `*.c` into `a.c b.c` before executing `echo`.
3. **Why did `ls alfa beta.txt` fail, and how can it be fixed?** Bash split the name at the space. Use `ls "alfa beta.txt"`, `ls 'alfa beta.txt'` or `ls alfa\ beta.txt`.
4. **What is the difference between single and double quotes?** Single quotes preserve text literally. Double quotes keep it together as one argument but allow variable and command substitution. `\` can escape the next character.

## Part 3 — Redirection, Chaining and Jobs

### Commands executed

```bash
cd ~/os-lab3
ls /etc /nope > out.txt 2> err.txt
wc -l out.txt ; cat err.txt
ls /etc /nope > all.txt 2>&1 ; tail -2 all.txt
echo first > notes.md ; echo second >> notes.md ; cat notes.md
wc -l < access.log
ls /nope 2> /dev/null || echo "failed with code $?"
mkdir -p tmp && cd tmp && echo "I am in $(pwd)" ; cd ~/os-lab3
sleep 100 &
jobs ; kill %1 ; jobs
```

### Predictions, outputs and explanations

| Test | Prediction | Output observed in the report |
|---|---|---|
| `ls /etc /nope > out.txt 2> err.txt` | Directory listing goes to stdout file; error to stderr file. | `out.txt` had **237 lines**; `err.txt` contained `ls: cannot access '/nope': No such file or directory`. |
| `ls /etc /nope > all.txt 2>&1` | Both output streams go into `all.txt`. | Directory listing and error were together in `all.txt`. |
| `ls /etc /nope 2>&1 > all.txt` | Error remains on terminal; directory listing goes to file. | Error displayed on screen. |
| `echo first > notes.md ; echo second >> notes.md ; cat notes.md` | First redirect overwrites, second appends. | Two lines: `first` and `second`. |
| `wc -l < access.log` | Count lines without showing filename. | `500`. |
| `ls /nope 2> /dev/null || echo "failed with code $?"` | Hide error and print the failure code. | `failed with code 2`. |
| `mkdir -p tmp && cd tmp && echo "I am in $(pwd)"` | Enter `tmp` if creation succeeds and print current directory. | `I am in /home/gabi/os-lab3/tmp`. |
| `sleep 100 & ; jobs ; kill %1 ; jobs` | Start a background process, list it, terminate it. | Job `[1]` appeared as `Running`, then `Terminated`. The PID is run-specific. |

### Observe — answers

1. **Where did stdout and stderr go?** The listing of `/etc` went to `out.txt` (file descriptor 1), while the error for `/nope` went to `err.txt` (file descriptor 2). `wc -l out.txt` displayed **237**.
2. **Why do `> all.txt 2>&1` and `2>&1 > all.txt` differ?** Bash applies redirections **left to right**. In the first case, both streams point to the file. In the second, stderr stays linked to the terminal while stdout moves to the file.
3. **Why does `wc -l < access.log` display just `500`?** The shell connects the file to standard input, so `wc` is given no filename argument. `wc -l access.log` instead displays `500 access.log`.
4. **What is the difference between `;`, `&&` and `||`?** `;` always starts the next command after the previous one finishes. `&&` runs the next only if the previous command succeeded. `||` runs it only if the previous command failed.

## Questions Q1–Q8 — Answers with One-Liners

Each record in `access.log` is in the format `date time user result page`. The laboratory dataset contains **500 requests**.

### Q1. How many requests failed?

```bash
grep -c FAIL access.log
```

**Output:** `100`. Every fifth request failed.

### Q2. How many different pages were requested, and which?

```bash
cut -d' ' -f5 access.log | sort -u ; cut -d' ' -f5 access.log | sort -u | wc -l
```

**Output:** `/page0`, `/page1`, `/page2`, `/page3`; **4** distinct pages.

### Q3. How many requests did each page get?

```bash
cut -d' ' -f5 access.log | sort | uniq -c
```

**Output:** `/page0`: 125, `/page1`: 125, `/page2`: 125, `/page3`: 125. Total: **500**.

### Q4. Which user or users have the most failed requests, and how many?

```bash
grep FAIL access.log | cut -d' ' -f3 | sort | uniq -c | sort -rn | head -2
```

**Output:** `user3` and `user5`, **15 failures each**. The five other users have **14** each.

### Q5. How many requests did user3 make, and how many of them failed?

```bash
grep -c ' user3 ' access.log ; grep ' user3 ' access.log | grep -c FAIL
```

**Output:** **72** total requests, **15** failures.

### Q6. Print the last 3 failed requests, showing only time and user.

```bash
grep FAIL access.log | tail -3 | cut -d' ' -f2,3
```

**Output:**

```text
10:10 user0
10:15 user5
10:20 user3
```

### Q7. Which login shells appear in /etc/passwd, and how many accounts use each?

```bash
cut -d: -f7 /etc/passwd | sort | uniq -c | sort -rn
```

**Output recorded in the report screenshot:**

```text
40 /usr/sbin/nologin
5 /bin/false
2 /bin/bash
1 /bin/sync
```

Most system accounts use `/usr/sbin/nologin`; two accounts use `/bin/bash`.

### Q8. Predict, then explain: why do `ls /etc | wc -l` and `ls -l /etc | wc -l` differ by exactly one?

```bash
ls /etc | wc -l ; ls -l /etc | wc -l
```

**Prediction:** Long-format `ls -l` adds one line.

**Output:** `236`, then `237`.

**Explanation:** When writing to a pipe, regular `ls` prints one filename per line. `ls -l` prints the same entries but adds a heading of the form `total N`, so it produces one extra line.

## Conclusion

The exercises showed how Bash searches for programs, expands command-line patterns, handles quoting, redirects standard streams, chains commands according to exit status and controls background jobs. The one-liners used filters to extract and count data from a 500-request log, obtaining consistent totals (100 failures and four pages with 125 requests each).
