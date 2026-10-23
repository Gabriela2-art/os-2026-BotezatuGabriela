# Laboratory Work No. 5 — The File System with Your Own Hands

**Student:** Botezatu Gabriela  
**Group:** FAF-243  
**Subject:** Operating Systems  
**Environment:** Ubuntu 24.04 LTS, user `gabi`, working directory `~/os-lab5`

This document collects the **Observe** answers and the commands and outputs supporting them from the laboratory report.

## Part 1 — The Tree and the File Types

### Commands

```bash
whoami
mkdir -p ~/os-lab5 && cd ~/os-lab5
ls /
ls -ld /etc /etc/passwd /bin /dev/null /tmp
ls -l /dev | head
file /usr/bin/ls /etc/passwd /bin
du -sh /var/log 2>/dev/null ; du -sh /usr 2>/dev/null
```

### Observe 1 — Identify the file types

| Path | `ls -ld` prefix | Type |
|---|---|---|
| `/etc` | `drwxr-xr-x` | Directory (`d`) |
| `/etc/passwd` | `-rw-r--r--` | Regular file (`-`) |
| `/bin` | `lrwxrwxrwx` | Symbolic link (`l`) |
| `/dev/null` | `crw-rw-rw-` | Character device (`c`, major 1, minor 3) |
| `/tmp` | `drwxrwxrwt` | Directory (`d`) with sticky bit (`t`) |

### Observe 2 — What is `/bin`?

On Ubuntu 24.04, `/bin` is a symbolic link to `usr/bin` (which resolves to `/usr/bin`). This is the *merged `/usr`* layout: executables live under `/usr/bin` while `/bin` remains for compatibility with old paths such as `/bin/bash`.

**Supporting output:** `file /bin` identifies a symbolic link to `usr/bin`; `file /usr/bin/ls` identifies an x86-64 ELF executable, and `file /etc/passwd` identifies ASCII text.

### Observe 3 — Character versus block devices

`/dev/null` is a character device: its listing starts with `c` and reports device numbers `1, 3` instead of a normal size. The report also identifies `autofs` and `console` as character devices. Block devices have the prefix `b` (for example a disk represented under `/dev/block`, with major/minor numbers such as `8, 0`). Character devices transfer byte streams; block devices provide block-oriented storage suitable for file systems.

### Observe 4 — Compare `/var/log` and `/usr`

**Recorded output:** `/var/log`: **494M**; `/usr`: **5.3G**. `/usr` was approximately 11 times larger because it contains installed programs and libraries, while `/var/log` stores system logs.

## Part 2 — Inodes and Links

### Commands and observed results

```bash
echo hello > notes.txt
stat notes.txt
ls -li notes.txt
ln notes.txt hard.txt
ls -li notes.txt hard.txt
rm notes.txt
cat hard.txt ; ls -li hard.txt
ln -s hard.txt soft.txt
ls -li hard.txt soft.txt
cat soft.txt
rm hard.txt
cat soft.txt ; ls -l soft.txt
mkdir d ; ls -ld d
mkdir d/x d/y ; ls -ld d
echo x > one.txt ; ls -l one.txt ; du -h one.txt
truncate -s 1G big.img ; ls -lh big.img ; du -h big.img
rm big.img
```

**Outputs documented in the report:**

- `stat notes.txt`: size **6 bytes**, link count **1**, permissions `0664` (`-rw-rw-r--`), owner `gabi`; `ls -li` displayed the same inode as `stat`.
- After `ln notes.txt hard.txt`, both names pointed to the same inode and had link count **2**. After `rm notes.txt`, `cat hard.txt` still printed `hello`, and its link count was **1**.
- `soft.txt -> hard.txt` was an **8-byte symbolic link**, with its own inode. While the target existed, `cat soft.txt` printed `hello`; after `rm hard.txt`, `cat soft.txt` reported **No such file or directory**.
- `ls -ld d` showed link count **2** before and **4** after creating `d/x` and `d/y`.
- `one.txt` had a logical size of **2 bytes** but `du -h` showed **4.0K**. `big.img` had a logical size of **1.0G** but `du -h` showed **0**.

### Observe 1 — Inode and link count

`stat` reports an **Inode** number, also shown in the first column of `ls -li`. The **Links** field counts directory entries referring to that inode; a newly created file has one link. The report's initial `stat` description and screenshot show inode **919774** (a later Observe paragraph prints **919744**, which appears to be a transcription inconsistency).

### Observe 2 — Why could `hard.txt` still be read?

`notes.txt` and `hard.txt` were two names of the same inode, with no special “original” name. `rm notes.txt` removed one directory entry through `unlink()` and reduced the count from 2 to 1. The data stayed available through `hard.txt`; it is freed only after there are no hard links and no open references.

### Observe 3 — Why did `cat soft.txt` fail?

A symbolic link stores the **path** to its target, not the target inode. After `hard.txt` was removed, the link still existed but pointed to a missing name, becoming a *dangling symbolic link*. The text `hard.txt` is eight bytes, matching the symbolic link's reported size.

### Observe 4 — Why did directory links increase from 2 to 4?

A new directory has two links: its entry in the parent directory and its own `.` entry. Every immediate subdirectory has a `..` entry pointing to that parent. Therefore `d/x` and `d/y` add two links: **2 + 2 = 4**.

### Observe 5 — Why does a 2-byte file occupy 4K, while 1 GB occupies zero?

The file system allocates space in blocks of **4096 bytes**, so a two-byte file occupies a 4K block. `truncate -s 1G` creates a **sparse file**: it records a 1 GB logical size without allocating the unwritten data blocks. `ls` shows logical file size, whereas `du` reports actually allocated disk space.

## Part 3 — Permissions

### Commands and outputs

```bash
echo secret > s.txt
chmod 600 s.txt ; ls -l s.txt
chmod 000 s.txt ; cat s.txt
chmod u+r s.txt ; cat s.txt
umask ; touch new.txt ; mkdir newdir ; ls -ld new.txt newdir
mkdir box ; echo hi > box/f
chmod u=r,go= box ; ls box ; cat box/f
chmod u=x,go= box ; ls box ; cat box/f
chmod 755 box
ls -l /usr/bin/passwd ; ls -ld /tmp
cat /etc/shadow
ls -l /etc/shadow
```

Recorded results: `chmod 600` yielded `-rw-------`; `chmod 000` made `cat s.txt` fail with `Permission denied`; after `chmod u+r`, it printed `secret`. `umask` printed `0002`; `new.txt` had `-rw-rw-r--` and `newdir` had `drwxrwxr-x`.

### Observe 1 — Convert octal permissions to `rwx`

| Octal | `rwx` | Typical use from the report |
|---|---|---|
| `600` | `rw-------` | A private file, e.g. `~/.ssh/id_rsa` |
| `644` | `rw-r--r--` | Ordinary readable file, e.g. `/etc/passwd` |
| `755` | `rwxr-xr-x` | Public executable or directory, e.g. `/usr/bin/ls` |
| `700` | `rwx------` | Private directory or script, e.g. `~/.ssh` |

### Observe 2 — Why does ownership not bypass `chmod 000`?

The kernel selects the appropriate permission class—owner, group, or others—and checks its bits. Because `gabi` owns `s.txt`, the owner bits `---` apply and reading is denied; permissions from another class are not substituted. The owner can still change permissions with `chmod`, so `chmod u+r` restored read access.

### Observe 3 — What does `umask 0002` do?

`0002` removes write permission for **others**. New files start with a maximum of `666`, yielding **664 (`rw-rw-r--`)**; new directories start with `777`, yielding **775 (`rwxrwxr-x`)**. The observed objects were `new.txt` and `newdir` with those exact modes.

### Observe 4 — What is the difference between `r` and `x` on a directory?

| Directory permission | Command | Prediction | Actual result |
|---|---|---|---|
| `r--` | `ls box` | `f` | `f`, plus `ls: cannot access 'box/f': Permission denied` |
| `r--` | `cat box/f` | Permission denied | `cat: box/f: Permission denied` |
| `--x` | `ls box` | Permission denied | `ls: cannot open directory 'box': Permission denied` |
| `--x` | `cat box/f` | `hi` | `hi` |

`r` permits reading the directory's list of names; `x` permits traversing the directory to access a known filename. Ubuntu's colored `ls` also attempts to inspect file metadata, which explains its error when it could list `f` but could not traverse into `box`.

### Observe 5 — What do the `s` and `t` mean?

**Supporting output:** `/usr/bin/passwd`: `-rwsr-xr-x root root`; `/tmp`: `drwxrwxrwt root root`; `/etc/shadow`: `-rw-r----- root shadow`. `cat /etc/shadow` produced **Permission denied**.

The `s` in `passwd` is **setuid**: it runs with the effective UID of its owner (`root`), letting it make controlled password changes despite ordinary users lacking access to `/etc/shadow`. The `t` in `/tmp` is the **sticky bit**, which prevents users in the shared directory from deleting or renaming files belonging to other users (except administrators / authorized owners).

## Part 4 — Disks and the Kernel as Files

### Commands

```bash
lsblk
df -h
mount | grep " / "
grep -c processor /proc/cpuinfo
head -3 /proc/meminfo
head -5 /proc/self/status
ls -l /proc/self/fd
```

### Observe 1 — Disk, partitions and root filesystem

The VM disk was **`/dev/sda` (25G)** with `sda1` (1M, BIOS boot) and `sda2` (25G). The root filesystem was mounted from **`/dev/sda2`** at `/`, as **ext4**. The mount command reported `/dev/sda2 on / type ext4 (rw,relatime)`. Loop devices represented read-only snap package images mounted under `/snap`; `sr0` represented the virtual optical drive.

### Observe 2 — How full was `/`?

`df -h` showed **25G total**, **8.9G used**, **15G available**, or **38% used**.

### Observe 3 — CPU and RAM

`grep -c processor /proc/cpuinfo` printed **2** logical processors. `MemTotal` in `/proc/meminfo` was **3480360 kB** (approximately **3.3 GiB**). `MemFree` and `MemAvailable` described free and reclaimable memory. These are resources visible to the VM, not necessarily the whole host machine.

### Observe 4 — Why did `/proc/self/status` say `Name: head`?

`/proc/self` resolves dynamically to the process opening it. Since `head` read `/proc/self/status`, the name was **`head`**, not `bash`. Similarly, `ls -l /proc/self/fd` showed file descriptors of `ls`: **0, 1, 2** attached to `/dev/pts/0`, and **3** pointing to the `/proc/PID/fd` directory that `ls` opened.

## Optional Stretch Tasks Completed

### (a) Magic numbers — a gzip file called `notes.txt`

```bash
gzip -c /etc/passwd > notes.txt
file notes.txt
xxd notes.txt | head -1
```

Despite the `.txt` extension, `file` identified **gzip compressed data**, from the original file `passwd`. The first bytes were **`1f 8b 08 08`**: `1f 8b` is the gzip magic number, `08` the deflate compression method, and the next `08` a flag indicating the original name is included. Linux file identification can inspect file contents rather than rely on filename extensions.

### (c) Deleted file still open

```bash
head -c 100M /dev/zero > bigfile ; df -h ~
exec 3< bigfile ; rm bigfile
ls -l /proc/$$/fd ; df -h ~
exec 3<&- ; df -h ~
```

After `rm bigfile`, the filename disappeared, but descriptor **3** still showed `/home/gabi/os-lab5/bigfile (deleted)` in `/proc/$$/fd`. The used disk space did **not** fall until `exec 3<&-` closed the descriptor, when about **100 MB** was released. An unlinked inode remains allocated until no directory links **and** no open references remain.
