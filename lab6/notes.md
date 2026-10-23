# Laboratory Work No. 6 — Inside the xv6 File System

**Course:** Operating Systems (FCIM / FAF / UTM, 2026–2027)  
**Student:** Botezatu Gabriela, FAF-243  
**Environment:** Ubuntu 24.04 LTS, `~/xv6-riscv`, QEMU (RISC-V)

This file records the **Observe** answers, commands and supporting outputs documented in `Lab6_BotezatuGabriela_FAF243.docx`. Terminal screenshots and full source listings are in the Word report. The requested deliverables also include `user/mystat.c`, `user/find.c` and the Makefile with their `UPROGS` entries.

## Preparation

The two user programs were included in the Makefile's `UPROGS` list:

```make
$U/_mystat\
$U/_find\
```

The xv6 file system was tested after a clean build:

```bash
cd ~/xv6-riscv
make clean
make qemu
```

## Part 1 — Reading What xv6 Shows

### Commands and outputs

At the xv6 `$` prompt:

```text
$ ls
.       1 1 1024
..      1 1 1024
README  2 2 2441
...
console 3 25 0
$ mkdir a
$ ls a
.       1 26 32
..      1 1 1024
```

*The root listing above is abbreviated; inode numbers of newly created files can depend on the filesystem image and the order of commands.*

### Observe — answers

1. **File types 1, 2 and 3.** In `kernel/stat.h`, `T_DIR = 1` means directory, `T_FILE = 2` means regular file, and `T_DEVICE = 3` means device file. Thus `.` and `..` have type `1`, `README` and the executable programs have type `2`, and `console` has type `3`. The `console` size is `0`, since device input/output is handled by its driver rather than stored in ordinary file data blocks.

2. **Root inode.** The root has inode **1** (`ROOTINO` in `kernel/fs.h`). Inode `0` denotes an unused directory entry. Both `.` and `..` in `/` point to inode `1`, because the root is its own parent.

3. **Size of an empty directory.** Each `struct dirent` consists of a 2-byte inode number and a 14-byte name (`DIRSIZ = 14`), so an entry occupies **16 bytes**. The new directory `a` contains `.` and `..`, hence its size is **2 × 16 = 32 bytes**. Its `.` points to inode `26` and its `..` to root inode `1` in the recorded run. The original root directory occupies **1024 bytes**, as created by `mkfs`.

## Part 2 — `mystat`, Links and On-Disk Inodes

### Commands and outputs

The program `user/mystat.c` uses `stat()` to print file type, inode, hard-link count and size:

```text
$ mystat README . console
README: file, inode 2, links 1, size 2441 bytes
.: directory, inode 1, links 1, size 1024 bytes
console: device, inode 26, links 1, size 0 bytes

$ ln README r2
$ mystat README r2
README: file, inode 2, links 2, size 2441 bytes
r2: file, inode 2, links 2, size 2441 bytes
$ rm README
$ mystat r2
r2: file, inode 2, links 1, size 2441 bytes
$ cat r2 | wc
48 336 2441
```

The sample `console` inode is from the separate `mystat` run recorded in the report; it differs from the initial root listing because the tests were run on different filesystem states.

### Observe — answers

1. **After `ln` and `rm`.** Both `README` and `r2` have inode **2**, size **2441 bytes**, and link count **2** after `ln README r2`. After `rm README`, `r2` still has inode `2` and size `2441`, but its link count returns to **1**. The output `48 336 2441` confirms that the original content remains readable. Hard links are multiple directory names for one inode.

2. **Where the link count changes and blocks are freed.** In `kernel/sysfile.c`, `sys_unlink()` removes the directory entry and performs `ip->nlink--`, then calls `iupdate(ip)`. In `kernel/fs.c`, `iput()` checks whether the inode has no links and only the final in-memory reference remains (`ip->nlink == 0` and `ip->ref == 1`). It then calls `itrunc()` to free data blocks and marks the inode free. Removing only one of several names does **not** free the data blocks.

3. **Fields of `struct dinode`.** The on-disk inode in `kernel/fs.h` has `type`, `major`, `minor`, `nlink`, `size`, and `addrs[NDIRECT + 1]`. Their sizes are 2, 2, 2, 2, 4 and 52 bytes respectively: **64 bytes** total, or **16 inodes per 1024-byte block**. Unlike an ext4 inode, it has no owner ID, permission bits, timestamps or separate symbolic-link type.

| Field | Size | Meaning |
|---|---:|---|
| `type` | 2 bytes | Directory, file, device or free inode |
| `major`, `minor` | 2 + 2 bytes | Device numbers |
| `nlink` | 2 bytes | Number of directory links |
| `size` | 4 bytes | File size in bytes |
| `addrs[13]` | 52 bytes | 12 direct pointers and 1 indirect pointer |

## Part 3 — The Limits of the File System

### Commands and outputs

On Ubuntu, in the xv6 source tree:

```bash
grep -n "define BSIZE\|define NDIRECT\|define NINDIRECT\|define DIRSIZ" kernel/fs.h
make clean ; make fs.img 2>&1 | grep nmeta
```

The report records these constants and `mkfs` output:

```text
BSIZE       1024
NDIRECT     12
NINDIRECT   (BSIZE / sizeof(uint)) = 256
DIRSIZ      14
nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000
```

In xv6:

```text
$ mkdir averyveryverylongname
$ ls
...
averyveryveryl 1 26 32
```

### Observe — answers

1. **Long names.** `averyveryverylongname` appears as `averyveryveryl`, exactly **14 characters**. `DIRSIZ = 14` in `kernel/fs.h` limits the `name` field of each directory entry. `skipelem()` truncates longer pathname components. The xv6 version used in the report does not signal an error for this experiment.

2. **Maximum file size.** There are `NDIRECT = 12` direct data block pointers plus one indirect block containing `NINDIRECT = 1024 / 4 = 256` pointers. Thus `MAXFILE = 12 + 256 = 268` data blocks. With `BSIZE = 1024` bytes, the maximum is **268 × 1024 = 274,432 bytes (268 KiB)**.

3. **Disk layout.** The filesystem image has **2000 blocks of 1024 bytes**, or approximately **2 MiB**. The `mkfs` output shows **47 metadata blocks** and **1953 data blocks**.

| Disk area | Blocks | Explanation |
|---|---:|---|
| Boot | 1 | Boot block |
| Superblock | 1 | Filesystem parameters |
| Log / journal | 31 | One header and 30 log blocks |
| Inodes | 13 | Storage for 200 inodes |
| Bitmap | 1 | Allocation map |
| Data | 1953 | File and directory contents |
| **Total** | **2000** | **47 metadata + 1953 data** |

## Part 4 — The `find` Program

The `user/find.c` program recursively walks directories to print paths whose last component matches the requested name. It skips empty directory entries (`inum == 0`) as well as `.` and `..`, and closes opened descriptors to avoid exhausting the per-process `NOFILE = 16` limit.

### Commands and outputs

On a fresh xv6 filesystem:

```text
$ mkdir a
$ mkdir a/b
$ echo x > a/b/r2
$ echo y > a/r2
$ ln cat r2
$ find . r2
./a/b/r2
./a/r2
./r2
$ find a b
a/b
```

**Result:** The recursive search finds matching file names at different depths, and it can match directories as well as files.

## Stretch Task — `find -n`

The optional `-n` argument prints the inode number next to each matching path.

```text
$ find . r2 -n
./a/b/r2 30
./a/r2 31
./r2 3
```

**Observe:** Only `./r2` is the same file as the `cat` executable, because both have **inode 3**: `ln cat r2` created a hard link. The two nested files `a/b/r2` and `a/r2` were created independently and have different inode numbers. The Word report's prose gives alternative inode values for those two newly created files (28 and 29); the values shown here follow its terminal screenshot. The conclusion does not change.

## Conclusion

The experiments demonstrated that xv6 represents files through inodes, stores directory names in fixed-size entries and implements hard links by letting several names point to the same inode. `mystat` exposed inode metadata, while reading `sys_unlink()` and `iput()` explained when link counts decrease and data blocks are reclaimed. The limits of file names, files and the disk follow directly from `kernel/fs.h`. Finally, `find` demonstrated recursive directory traversal and its optional `-n` flag showed how inode numbers identify hard-linked files.
