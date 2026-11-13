
# Operating Systems — Session 3

## Why Does My Laptop Freeze When One App Goes Crazy?

### Lab A — CPU Scheduling Algorithms

This experiment compares three CPU scheduling algorithms: FIFO, SJF, and Round Robin.

Three jobs arrive at time 0:

- Job A: 100 CPU units
- Job B: 10 CPU units
- Job C: 10 CPU units

The objective is to compare their turnaround and response times.

| Algorithm | Average Turnaround | Average Response |
|---|---:|---:|
| FIFO | 110.00 | 70.00 |
| SJF | 50.00 | 10.00 |
| Round Robin (q=10) | 56.67 | 10.00 |
| Round Robin (q=1) | 59.67 | 1.00 |

SJF produces the best average turnaround time because it executes the shortest jobs first.

Round Robin with a quantum of 1 gives the best average response time, but it requires more frequent context switches.

A real processor incurs overhead during context switching because it must save and restore execution state.

FIFO and SJF produce the same schedule when the jobs are already arranged in nondecreasing order of CPU burst length.

In the scheduler simulator, an unfinished Round Robin process is placed at the back of the ready queue using queue.append(j).

#### Scheduling Timelines and Individual Results

All three jobs arrive at time 0. Job A requires 100 CPU units, while jobs B and C require 10 units each.

**FIFO — First In, First Out**

Timeline:

0 --- A --- 100 --- B --- 110 --- C --- 120

| Job | First execution | Completion | Response | Turnaround |
|---|---:|---:|---:|---:|
| A | 0 | 100 | 0 | 100 |
| B | 100 | 110 | 100 | 110 |
| C | 110 | 120 | 110 | 120 |

Average response = (0 + 100 + 110) / 3 = 70.

Average turnaround = (100 + 110 + 120) / 3 = 110.

**SJF — Shortest Job First**

Timeline:

0 --- B --- 10 --- C --- 20 --- A --- 120

| Job | First execution | Completion | Response | Turnaround |
|---|---:|---:|---:|---:|
| A | 20 | 120 | 20 | 120 |
| B | 0 | 10 | 0 | 10 |
| C | 10 | 20 | 10 | 20 |

Average response = (20 + 0 + 10) / 3 = 10.

Average turnaround = (120 + 10 + 20) / 3 = 50.

**Round Robin — Quantum 10**

Timeline:

0 --- A --- 10 --- B --- 20 --- C --- 30 --- A --- 120

After time 30, only job A remains, so it continues until completion.

| Job | First execution | Completion | Response | Turnaround |
|---|---:|---:|---:|---:|
| A | 0 | 120 | 0 | 120 |
| B | 10 | 20 | 10 | 20 |
| C | 20 | 30 | 20 | 30 |

Average response = (0 + 10 + 20) / 3 = 10.

Average turnaround = (120 + 20 + 30) / 3 = 56.67.

**Round Robin — Quantum 1**

Jobs A, B, and C alternate in one-unit time slices until B and C finish. Job A then completes its remaining work.

| Job | First execution | Completion | Response | Turnaround |
|---|---:|---:|---:|---:|
| A | 0 | 120 | 0 | 120 |
| B | 1 | 29 | 1 | 29 |
| C | 2 | 30 | 2 | 30 |

Average response = (0 + 1 + 2) / 3 = 1.

Average turnaround = (120 + 29 + 30) / 3 = 59.67.


### Lab B — CPU Overload and Scheduling

My Ubuntu WSL2 environment has 12 logical CPUs.

I started 13 CPU-intensive processes using the yes command. All 13 processes appeared in state R.

During the experiment, vmstat consistently reported r = 13. After the initial sample, CPU idle time was 0%.

Since only 12 logical CPUs were available, at least one of the 13 CPU-intensive processes had to wait for CPU time while all remained runnable.

I measured the execution time of ls -R /usr under two conditions.

| Measurement | With CPU hogs | Without CPU hogs |
|---|---:|---:|
| Real time | 18.659 s | 9.529 s |
| User time | 0.395 s | 0.199 s |
| System time | 1.750 s | 0.868 s |

The elapsed time was approximately 1.96 times longer under CPU overload.

This demonstrates how CPU contention can reduce system responsiveness. The total CPU utilization cannot exceed the capacity of the available logical CPUs.

I also tested Linux job control using Ctrl+Z, jobs, bg, and fg.

Pressing Ctrl+Z stopped the yes process, placing it in state T. Running bg resumed it in the background, where it returned to state R. Running fg brought it back to the foreground, and Ctrl+C terminated it.

Finally, I verified that no yes processes remained running.

### Lab C — Measuring CPU Fairness with Nice

I compiled spin.c and used taskset to restrict the experiments to logical CPU 0.

When one process ran alone, it completed approximately 12,000 to 14,000 units of work per second during most of the measurement.

When two processes with nice 0 competed for the same CPU, both completed approximately 7,000 units of work per second.

This demonstrated approximately equal CPU allocation between processes of equal scheduling priority.

I then compared a process with nice 0 against another with nice 10.

During seconds 6 to 8, the nice 0 process completed 40,677 units of work, while the nice 10 process completed 4,379 units.

The measured work ratio was approximately 9.29:1.

This closely matches the predicted CPU scheduling weight ratio of approximately 9 to 10.

The experiment demonstrates that CPU fairness does not always mean equal CPU time. Linux distributes CPU time according to scheduling weights influenced by nice values.

Ordinary users cannot freely increase process priority because this could allow them to monopolize CPU resources and negatively affect other users.

#### CPU Fairness Measurements

I compared two CPU-bound processes pinned to CPU 0 using taskset. I calculated their work ratios using the last three measurement intervals.

| Nice values | Measured ratio | Predicted ratio |
|---|---:|---:|
| 0 vs 0 | 1.01:1 | 1:1 |
| 0 vs 5 | 3.05:1 | About 3:1 |
| 0 vs 10 | 9.29:1 | About 9–10:1 |
| 0 vs 19 | 66.06:1 | About 70:1 |

These measurements are consistent with Linux's weighted CPU scheduling.

When I attempted to run a process at nice -5 without elevated privileges, Ubuntu displayed a permission-denied warning. When I repeated the command using sudo, it executed without that warning.

Ordinary users cannot freely increase scheduling priority because doing so could unfairly consume shared CPU resources.

#### Voluntary and Involuntary Context Switching

I started two CPU-bound processes on CPU 0 and monitored PID 1184 using watch.sh.

The voluntary context-switch counter remained at 1, while the involuntary counter increased from 6 to 896.

This increase of 890 involuntary context switches demonstrates scheduler preemption.

Voluntary switches occur when a process yields or blocks, while involuntary switches occur when the scheduler takes the CPU away from a runnable process.

### Lab D — Linux Signals

I compiled and executed catch.c to investigate how Linux processes respond to signals.

#### SIGINT

The first process had PID 1316.

I pressed Ctrl+C repeatedly. Instead of terminating, the program displayed:

Nice try! (SIGINT)

This happened because the program installed a signal handler for SIGINT.

#### SIGTERM

I sent SIGTERM to PID 1316.

The program displayed:

Asked politely (SIGTERM), cleaning up...

The exit status was 0 because the handler explicitly called _exit(0).

SIGTERM allows a process to perform cleanup before terminating when it has an appropriate handler.

#### SIGKILL

I restarted the program with PID 1366 and sent SIGKILL.

The terminal displayed:

Killed

The exit status was 137.

This is calculated as 128 + 9, where 9 is the number assigned to SIGKILL.

The signal handler did not execute because SIGKILL cannot be caught or ignored.

#### SIGSTOP and SIGCONT

I started another process with PID 1397.

After sending SIGSTOP, the process entered state T.

After sending SIGCONT, the process returned to state S.

This demonstrates that Linux can suspend and resume a process without terminating it.

#### Why SIGKILL and SIGSTOP Cannot Be Caught

The kernel reserves SIGKILL and SIGSTOP so that it can terminate or suspend processes even when those processes ignore other signals.

#### Other Signals

SIGSEGV (11): Usually generated when a program performs an invalid memory access.

SIGCHLD (17): Sent when a child process changes state, such as terminating or stopping.

SIGPIPE (13): Usually generated when a process writes to a pipe whose reading end has been closed.

#### Why Use write() Instead of printf() in a Signal Handler?

write() is async-signal-safe, while printf() is not.

printf() uses internal buffering and may access shared library state. If a signal interrupts code that is already using this state, calling printf() from the handler can cause undefined behavior or deadlock.

Therefore, write() is safer inside signal handlers.

### Bug Hunt — The Program That Would Not Stop

I compiled and executed deaf.c to investigate a process that ignores termination signals.

The program started with PID 1579.

When I pressed Ctrl+C, the process continued executing.

I inspected its signal configuration using:

grep -E 'SigIgn|SigCgt' /proc/1579/status

The output was:

SigIgn: 0000000000004002
SigCgt: 0000000000000000

The hexadecimal value 0x4002 has bits 1 and 14 set, corresponding to SIGINT (2) and SIGTERM (15).

This explains why the program ignored both signals.

I sent SIGTERM to PID 1579, but the process remained alive in state S+.

I then sent SIGKILL, and the process disappeared from the process table.

SIGKILL successfully terminated the process because it cannot be caught or ignored.

However, SIGKILL does not allow a process to perform application-level cleanup, save unfinished work, or release resources gracefully.

This experiment demonstrated how Linux handles ignored signals and why the kernel reserves SIGKILL as a reliable termination mechanism.
