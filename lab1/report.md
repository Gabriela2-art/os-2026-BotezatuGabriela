# Laboratory Work No. 1 — The OS as a Resource Manager

**Course:** Operating Systems (2026–2027)  
**Student:** Botezatu Gabriela, FAF-243  
**Environment:** Ubuntu Linux virtual machine

## Objective

Observe how Linux manages four fundamental resources: files and directories, processes, memory, and devices/storage, using terminal commands and interpreting their outputs.

## Initial environment

```bash
mkdir -p ~/os-lab1 && cd ~/os-lab1
whoami
uname -a
uptime
```

**Observed:** The working user was `gabi`. The report records an x86_64 Ubuntu Linux virtual machine and an uptime of approximately 13 minutes at the initial check.

## Part 1 — Files and directories

### Commands

```bash
pwd
ls -la /
ls -la ~
cd /etc && ls | head
cd ~/os-lab1
mkdir demo && cd demo
echo "hello operating systems" > note.txt
cp note.txt copy.txt
mv copy.txt renamed.txt
ls -l
rm renamed.txt
ls -l note.txt
chmod 600 note.txt
ls -l note.txt
chmod 644 note.txt
```

### Observe — answers

1. **Owner and group:** The files created in `demo` belonged to user `gabi` and group `gabi`, as shown by `ls -l`.
2. **Permissions after `chmod 600`:** `-rw-------` means `-` = regular file; `rw-` = the owner can read and write but not execute; `---` = no group permissions; `---` = no permissions for others. After `chmod 644`, the permissions became `-rw-r--r--`.
3. **Two directories directly under `/`:** `/etc` stores system-wide configuration, and `/dev` contains special files representing devices.

**Relevant output recorded in the report:**

```text
-rw------- 1 gabi gabi ... note.txt
```

The experiments showed that Linux tracks file ownership and checks access through permission bits.

## Part 2 — Processes

### Commands

```bash
ps aux | head
ps aux | wc -l
top
sleep 300 &
jobs
ps -ef | grep sleep
kill <PID>            # replace <PID> with the actual process ID
sleep 300 & PID=$!
ls /proc/$PID/
cat /proc/$PID/status | head -20
kill $PID
```

### Observe — answers

1. **PID 1:** On the observed Ubuntu system, PID 1 was `systemd`, which initializes the system and manages services.
2. **Approximate number of processes:** `ps aux | wc -l` returned **201 lines**. Excluding the column-header line, approximately **200 processes** were listed.
3. **State of the sleeping process:** For the observed `sleep` process (PID **5769**), `/proc/5769/status` showed `State: S (sleeping)`. The process was waiting, not actively using CPU time.

**Relevant outputs recorded in the report:**

```text
ps aux | wc -l
201

State: S (sleeping)
```

This part demonstrated how to start, locate, inspect, and terminate a background process.

## Part 3 — Memory

### Commands

```bash
free -h
cat /proc/meminfo | head -6
sleep 300 & PID=$!
grep VmRSS /proc/$PID/status
kill $PID
```

### Observe — answers

1. **RAM total and free:** The report records `MemTotal: 3480360 kB` (approximately **3.3 GiB**), about **633 MiB free**, and about **2.2 GiB available**. Available memory includes reclaimable memory such as cache.
2. **Swap:** Swap is disk space that Linux can use to supplement physical RAM. The observed VM had **0 B swap** configured.
3. **Resident memory of `sleep`:** The test process showed **`VmRSS: 7856 kB`** (approximately **7.7 MiB**). This is relatively small, as `sleep` performs little work while waiting.

**Relevant outputs recorded in the report:**

```text
MemTotal: 3480360 kB
Swap:     0 B
VmRSS:    7856 kB
```

## Part 4 — Devices and storage

### Commands

```bash
df -h
lsblk
du -sh ~/os-lab1
ls -l /dev | head
mount | head
```

### Observe — answers

1. **Root file system:** `/` was mounted on **`/dev/sda2`**, a partition of the virtual disk `/dev/sda` (approximately **25 GB**).
2. **A device entry:** `/dev/console` is a character device representing the system console. The leading `c` in its long listing identifies it as a character device.
3. **“Everything is a file”:** Linux exposes many resources, including disks, terminals, and devices, through file-like interfaces, allowing programs to interact with them using common file operations.

**Relevant result recorded in the report:**

```text
/dev/sda2 mounted on /
```

Commands such as `lsblk`, `df -h`, and `mount` make it possible to inspect the underlying storage and mounted file systems.

## Conclusion

This laboratory demonstrated the four main resources managed by an operating system: **files**, **processes**, **memory**, and **devices/storage**. File ownership and permissions were inspected with `ls -l` and `chmod`; processes were examined with `ps`, `jobs`, and `/proc`; memory was measured with `free -h`; and devices and mounted file systems were identified with `lsblk` and `df -h`. By running these commands and studying their results, I gained a clearer understanding of how Linux organizes, monitors, and protects system resources.

---

**Source:** Adapted from my combined *Laboratory Work No. 1 and No. 2* Word report, specifically the Laboratory Work 1 section. Screenshots and the extended discussion remain in the `.docx` report.
