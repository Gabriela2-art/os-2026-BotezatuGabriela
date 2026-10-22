# Laboratory Work No. 5 — The File System with Your Own Hands

**Course:** Operating Systems (FCIM / FAF / UTM, 2026–2027)  
**Student:** Botezatu Gabriela, FAF-243  
**Environment:** Ubuntu 24.04 LTS virtual machine, Bash, regular user `gabi`  

> These notes summarize the commands, observations and answers documented in `Lab5_BotezatuGabriela_FAF243.docx`. The Word report contains the supporting terminal screenshots and full explanations.

## Objective

Explore Linux file types, inodes, hard and symbolic links, permissions, storage devices and the `/proc` virtual file system by running commands and interpreting their results.

## Preparation

```bash
whoami
mkdir -p ~/os-lab5 && cd ~/os-lab5
```

**Observed:** `whoami` returned `gabi`, a regular user. Permission experiments were not performed as root.

## Part 1 — The tree and file types

```bash
ls /
ls -ld /etc /etc/passwd /bin /dev/null /tmp
ls -l /dev | head
file /usr/bin/ls /etc/passwd /bin
du -sh /var/log 2>/dev/null ; du -sh /usr 2>/dev/null
```

### Observe

1. **File types:** `/etc` is a directory (`d`); `/etc/passwd` is a regular file (`-`); `/bin` is a symbolic link (`l`); `/dev/null` is a character device (`c`); `/tmp` is a directory (`d`) with a sticky bit (`t`).
2. **`/bin`:** On the tested system, `/bin` is a symbolic link to `usr/bin` (that is, `/usr/bin`). This is the merged-`/usr` layout.
3. **Device examples:** `/dev/null` is a character device. The disk appears as a block device (for example `/dev/sda`); a block device is identified by `b` in the permission/type field.
4. **Directory size comparison:** `/usr` occupied approximately **5.3 GB**, while `/var/log` occupied approximately **494 MB**. `/usr` was roughly eleven times larger.

**Additional result:** `file` identified `/usr/bin/ls` as an ELF executable and `/etc/passwd` as text. File format is detected from contents, not merely from filename extensions.

## Part 2 — Inodes and links

### 2.1 Inspecting an inode

```bash
echo hello > notes.txt
stat notes.txt
ls -li notes.txt
```

**Observed:** The file size was 6 bytes (`hello` plus newline), the initial link count was 1, and the inode number shown by `stat` matched the first field from `ls -li`. `stat` also showed the owner, permissions, data blocks and timestamps.

### 2.2 Hard links

```bash
ln notes.txt hard.txt
ls -li notes.txt hard.txt
rm notes.txt
cat hard.txt ; ls -li hard.txt
```

**Observed:** Both names referred to the same inode. The link count increased to 2 after `ln` and returned to 1 after `rm notes.txt`; `cat hard.txt` still printed `hello`. A hard link is another directory entry for the same inode. `rm` removes a name, not necessarily the underlying data.

### 2.3 Symbolic links

```bash
ln -s hard.txt soft.txt
ls -li hard.txt soft.txt
cat soft.txt
rm hard.txt
cat soft.txt ; ls -l soft.txt
```

**Observed:** `soft.txt` had its own inode and stored the path `hard.txt` (8 bytes). After removing `hard.txt`, `cat soft.txt` failed with **No such file or directory**, although the symbolic link itself still existed. This is a *dangling symbolic link*.

### 2.4 Links to directories

```bash
mkdir d ; ls -ld d
mkdir d/x d/y ; ls -ld d
```

**Prediction and observed result:** The new directory had 2 links; after creating two subdirectories it had 4. The count includes the entry in its parent and `.` in the directory itself, plus one `..` entry from each child directory.

### 2.5 File size versus disk space

```bash
echo x > one.txt ; ls -l one.txt ; du -h one.txt
truncate -s 1G big.img ; ls -lh big.img ; du -h big.img
rm big.img
```

**Observed:** `one.txt` had a logical size of 2 bytes but occupied a 4 KB disk block. `big.img` had a logical size of 1 GB but initially occupied 0 disk blocks because it was created as a sparse file.

### Observe — answers

1. `Inode:` in `stat` gives the inode number; `Links:` gives the number of hard links (directory entries) referencing it.
2. `hard.txt` remained readable because removing `notes.txt` only removed one name for an inode that still had another name.
3. `soft.txt` became dangling after its target disappeared. Its size was 8 bytes, the length of the saved string `hard.txt`.
4. A directory's link count is normally **2 + number of immediate subdirectories** on the tested file system.
5. Files use disk allocation units (here 4 KB blocks), whereas sparse-file holes do not initially require allocated data blocks.

## Part 3 — Permissions

### 3.1 Ordinary file permissions and umask

```bash
echo secret > s.txt
chmod 600 s.txt ; ls -l s.txt
chmod 000 s.txt ; cat s.txt
chmod u+r s.txt ; cat s.txt
umask ; touch new.txt ; mkdir newdir ; ls -ld new.txt newdir
```

**Observed:** `chmod 600` produced `-rw-------`; `chmod 000` prevented reading the file; `chmod u+r` restored reading. The reported umask was **0002**. Newly created objects had `-rw-rw-r--` (`664`) for `new.txt` and `drwxrwxr-x` (`775`) for `newdir`.

| Octal | Permission string | Example use |
|---|---|---|
| `600` | `rw-------` | Private file readable and writable only by its owner |
| `644` | `rw-r--r--` | Standard file publicly readable but owner-writable |
| `755` | `rwxr-xr-x` | Executable program or traversable public directory |
| `700` | `rwx------` | Private directory or owner-only executable script |

### 3.2 Permissions on directories

```bash
mkdir box ; echo hi > box/f
chmod u=r,go= box ; ls box ; cat box/f
chmod u=x,go= box ; ls box ; cat box/f
chmod 755 box
```

**Predicted and observed:** With directory permission `r--`, `ls box` could display the name `f` but also produced a metadata-access error, and `cat box/f` was denied. With permission `--x`, `ls box` could not list the directory, but `cat box/f` printed `hi` because the known path could be traversed.

### 3.3 Special bits and protected files

```bash
ls -l /usr/bin/passwd ; ls -ld /tmp
cat /etc/shadow
ls -l /etc/shadow
```

**Observed:** `/usr/bin/passwd` had the **setuid** bit (`s`) and `/tmp` had the **sticky** bit (`t`). Reading `/etc/shadow` as the normal user failed with **Permission denied**.

### Observe — answers

1. The permission strings for `600`, `644`, `755`, and `700` are shown in the table above.
2. Ownership alone does not override permissions. With `chmod 000`, the owner's permission bits deny reading, although the owner can change the mode again with `chmod`.
3. With umask `0002`, new regular files receive `664` and new directories receive `775` (starting from maximum default modes `666` and `777`, with masked bits removed).
4. Directory `r` permits listing names; directory `x` permits traversing the directory to reach an already known entry.
5. **setuid** lets `passwd` run with the file owner's effective privileges to update password information under controlled conditions. **sticky bit** prevents users from deleting or renaming other users' files in shared directories such as `/tmp` (subject to normal ownership and privilege exceptions).

## Part 4 — Disks and the kernel as files

```bash
lsblk
df -h
mount | grep " / "
grep -c processor /proc/cpuinfo
head -3 /proc/meminfo
head -5 /proc/self/status
ls -l /proc/self/fd
```

### Observe

1. **Disk structure:** The report records a 25 GB disk `/dev/sda`, with `/dev/sda1` (1 MB, BIOS boot) and `/dev/sda2` (root `/`, `ext4`). It also shows `sr0` (virtual optical drive) and loop devices used for Snap packages.
2. **Root file-system usage:** The `df -h` output in the report showed approximately **25 GB total, 8.9 GB used, 15 GB available, 38% used**.
3. **CPU and RAM:** `/proc/cpuinfo` reported **2 logical processors**, and `/proc/meminfo` reported **MemTotal: 3480360 kB** (about 3.3 GiB).
4. **`/proc/self`:** `head -5 /proc/self/status` showed `Name: head`, because `/proc/self` refers to the process opening the virtual file. Similarly, `ls -l /proc/self/fd` lists file descriptors of the `ls` process.

## Stretch tasks (optional, both completed)

### A — File signatures (magic numbers)

```bash
gzip -c /etc/passwd > notes.txt
file notes.txt
xxd notes.txt | head -1
```

**Observed:** `notes.txt` was recognized as compressed gzip data despite the `.txt` extension. Its header began with `1f 8b`, gzip's magic number; the next `08` indicates the deflate compression method.

### C — Deleting an open file

```bash
head -c 100M /dev/zero > bigfile ; df -h ~
exec 3< bigfile ; rm bigfile
ls -l /proc/$$/fd ; df -h ~
exec 3<&- ; df -h ~
```

**Observed:** After `rm`, descriptor 3 still pointed to `bigfile (deleted)` and the disk space was not yet released. The space was freed after closing descriptor 3 with `exec 3<&-`. Removing the last filename is not sufficient to reclaim data blocks while a process still holds the file open.

## Conclusion

In this laboratory work, I explored the main concepts of the Linux file system through practical experiments. The exercises helped me understand how files and directories are organized, how inodes store file information, and how hard links and symbolic links behave when files are removed.

I also learned how Linux permissions work using commands such as chmod and umask. The experiments demonstrated how access rights control files and directories, while special bits such as setuid and sticky bit provide additional security.

Furthermore, I examined disk partitions, mounted file systems, and system information available through /proc. The optional exercises showed how file types can be identified by their content and why deleting an open file does not immediately free its disk space.

Overall, this laboratory improved my practical understanding of Linux file systems, permissions, and storage management. It also strengthened my ability to use terminal commands, interpret their outputs, and explain how the operating system manages files internally.

