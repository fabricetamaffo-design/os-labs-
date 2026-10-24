# Operating Systems — Lab 1 Report

**Session:** 1 — What does the middleman actually do?  An operating system is the software that shares one machine’s hardware safely among many programs and gives them simple ways to use it. You know it as Windows, macOS, Android or Linux; its core is called the kernel.
**Lab environment:** Ubuntu on Windows Subsystem for Linux 2 (WSL2)  
**Date of observations:** 24 October 2026  

## Operating system definitions

**My definition before class:** [Write your actual original one-line definition here; do not invent what you wrote.]  

**My revised definition:** An operating system manages hardware resources, protects running programs, and provides common services. Its **kernel** handles privileged operations, and programs request those services through a **system call**.

The kernel acts as a **referee** by enforcing permissions, an **illusionist** by scheduling many processes across limited CPUs and managing virtual memory, and **glue** by providing interfaces to files, devices, and other resources.

## Lab A — Linux environment

I installed Ubuntu under WSL2 and installed the required tools. The verified versions were GCC 15.2.0, strace 6.19, and Git 2.53.0. I created the working directory `~/os-labs/lab1`.

## Lab B — Interrogate the machine

| Command / question | My observation and explanation |
|---|---|
| `uname -a`: kernel and architecture | `6.18.54.1-microsoft-standard-WSL2`, `x86_64`. |
| `lscpu`: CPU model and CPUs visible | AMD Ryzen 5 PRO 4650U with Radeon Graphics; 12 logical CPUs (6 cores, 2 threads per core). |
| `nproc`: does it match? | `12`; yes, it matches the 12 logical CPUs reported by `lscpu`. |
| `free -h`: available RAM and why it exceeds free RAM | Available RAM was approximately **6.9 GiB** (total 7.4 GiB). Available memory estimates memory that programs could use without significant swapping, including reclaimable cached memory; free memory counts memory that is currently unused. Both displayed as 6.9 GiB after rounding. |
| `df -h`: root filesystem and free space | `/` was mounted from `/dev/sdd`, with approximately **954 GiB available** (1007 GiB reported size). This is WSL2 virtual-disk reporting, not necessarily free physical space on Windows C:. |
| `ps aux \| head`: PID 1 | PID 1 was `/usr/lib/systemd/systemd`, the system/service manager. |
| `ps aux \| wc -l`: process count | 28 output lines, including the header, so **27 process rows** in that snapshot. This is more than 12 logical CPUs. |
| `htop`: highest CPU user | **[Fill in the process name you actually observed after sorting by CPU using F6.]** |

**Discussion:** There were more process rows than logical CPUs, but processes can still make progress because the kernel schedules runnable processes and switches CPU time among them. Many processes may also be sleeping rather than actively using a CPU. These measurements describe the WSL2 Linux environment, not all Windows processes.

## Lab C — Every process is a folder

I ran `echo $$`, `ls /proc/$$`, and `cat /proc/$$/status` to inspect my Bash shell.

| Field | Observed value | Meaning |
|---|---|---|
| Name | `bash` | Shell process |
| State | `S (sleeping)` | Waiting for an event/input |
| Pid | `462` | Shell's process ID at observation time |
| PPid | `460` | Parent process ID |
| Uid | `1000` | User identity checked by access-control rules |
| VmRSS | `5536 kB` | Resident memory (about 5.4 MiB) |
| Threads | `1` | One thread |
| voluntary_ctxt_switches | `716`, later `731` | Voluntary switches increased as Bash waited for events |

The executable reported by `readlink /proc/$$/exe` was `/usr/bin/bash`. File descriptors `0`, `1`, and `2` all pointed to `/dev/pts/2`: standard input, standard output, and standard error respectively.

### Process creation and termination

I started `sleep 300 &` in the background. The process had **PID 2427**, **PPid 462**, and **state `S (sleeping)`**, because it was waiting on a timer rather than actively computing. After `kill $!`, `ls /proc/$!` reported `No such file or directory`, showing that the process's `/proc` entry disappeared after termination. I repeated the exercise with PID 2530 and observed the same behavior.

### Machine information in `/proc`

`/proc/cpuinfo` reported the AMD Ryzen 5 PRO 4650U. `/proc/meminfo` reported `MemTotal: 7785260 kB` and `MemAvailable: 7232152 kB` in one observation. `/proc/uptime` returned `1241.99 14836.74`: the first number is system uptime in seconds and the second is aggregate idle time across CPUs.

`ls -l /proc/meminfo` showed a file size of `0`, yet its contents were readable. This is because `/proc` is a virtual filesystem: the kernel generates the information when it is read, and the reported file size is not the amount of readable data.

## Lab D — Watch the middleman work

### Original program and its system calls

The original `hello.c` printed `Hello, OS!`. I compiled it with `gcc -Wall -g hello.c -o hello` and inspected it with `strace ./hello`.

A key line was:

```text
write(1, "Hello, OS!\n", 11) = 11
```

Here `write` requests output, descriptor `1` is standard output, the string is the data, the requested byte count is `11`, and the result `= 11` means all 11 bytes were written. This demonstrates a user-space program asking the kernel for an output service.

Other observed calls included `execve` (execute the program), `openat` and `read` (access files and libraries), `mmap` and `brk` (memory management), and `exit_group` (process termination).

### Experiment 1 — Save and count the trace

I used `strace -o strace-hello.txt ./hello`, followed by `wc -l strace-hello.txt`, which reported **36 lines**. The later `strace -c` summary reported **34 system calls**; the trace text also contains an exit indication, so counting file lines is not exactly the same as counting system calls.

### Experiment 2 — System-call summary

Using `strace -c ./hello`, the most frequent system call was **`mmap` with 8 calls**. The summary showed **34 total system calls**.

### Experiment 3 — Three printf calls and output buffering

I edited the program to call `printf` three times, printing `First line`, `Second line`, and `Third line`.

- With `strace -e trace=write ./hello`, there were **3 `write` calls** (11, 12, and 11 bytes).
- With `strace -e trace=write ./hello > out.txt`, there was **1 `write` call** of **34 bytes**.

This difference illustrates C standard I/O buffering. Terminal output is typically line-buffered, so each newline causes a flush. When stdout is redirected to a regular file, it is typically fully buffered, so the three lines are combined into one write at normal program termination.

### Experiment 4 — Library calls versus system calls

I ran `ltrace ./hello` on the original program and observed `puts("Hello, OS!")`. `ltrace` traces library calls; `strace` traces system calls. The compiler may optimize a simple `printf` into `puts`.

### Experiment 5 — Compare with ls

`strace -c ls` reported **144 calls** compared with **34** for my original program, so `ls` made **110 more system calls** in those measurements. It performs extra work to examine directory contents and display entries.

### The referee says no — permission check

As a normal user, `cat /etc/shadow` displayed `Permission denied`. The trace showed:

```text
openat(AT_FDCWD, "/etc/shadow", O_RDONLY|O_CLOEXEC) = -1 EACCES (Permission denied)
```

The kernel refused the file-opening request because the process did not have sufficient permissions. This illustrates the OS's referee/protection role.

## Conclusion

These labs let me observe the operating system as a middleman. Linux exposes information about hardware and processes through commands and `/proc`; its scheduler shares CPU time; and programs request privileged services through system calls. `strace` made those requests visible, while the `/etc/shadow` experiment demonstrated the kernel enforcing access permissions.

## Final checks before submission

- [ ] Replace the placeholder with my real original OS definition from before class.
- [ ] Record the process I observed at the top of CPU-sorted `htop`.
- [ ] Keep the submitted `hello.c` consistent with the saved `strace-hello.txt` (original `Hello, OS!` program), optionally saving the three-line version separately.
- [ ] Ensure `hello.c`, `strace-hello.txt`, and `report.md` are inside `os-labs/lab1` and push them to GitHub.
