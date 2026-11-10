# Laboratory Work 11 — The Race and the Semaphore

**Course:** Operating Systems  
**Student:** Botezatu Gabriela  
**Group:** FAF-243  

## Objective

Observe a race condition when two Linux threads increment a shared counter, then solve it using a mutex, a semaphore, and an atomic operation. Demonstrate three semaphore roles: **lock** (initial value 1), **signal** (0), and **resource counter** (3).

**Environment:** Ubuntu 26.04 LTS in VirtualBox; GCC 15.2.0. Programs were compiled with `-Wall -pthread` and without `-O2`.

## Part 1 — See the race

**File:** `race.c`

**Build and run:**

```bash
gcc -Wall -pthread -o race race.c
./race
./race
./race
```

**Recorded results:**

```text
counter = 1205492 (expected 2000000)
counter = 1385888 (expected 2000000)
counter = 1568956 (expected 2000000)
```

The two threads perform one million increments each. Without synchronization, some increments are lost because `counter++` is not atomic.

### Observe: What are the three steps performed by `counter++`?

1. **Load** the current value of `counter` from memory.
2. **Add 1** to that value.
3. **Store** the updated value back in memory.

### Observe: How can two threads lose one addition?

If `counter` is 5, thread A and thread B can both load 5 before either stores a result. Both compute 6 and both store 6. Two increments were attempted, but the counter increased by only one. This is a **race condition**.

### Observe: Why is the result different at every run?

The Linux scheduler and the CPU interleave the threads differently each time. The number of overlapping read-modify-write operations therefore changes, producing different incorrect results.

## Part 2 — Fix it: mutex, then semaphore

### 2.1 — Mutex

**File:** `mutex.c`

```bash
gcc -Wall -pthread -o mutex mutex.c
./mutex
./mutex
./mutex
```

**Recorded result:** All three runs printed `counter = 2000000 (expected 2000000)`.

The `counter++` operation is protected by `pthread_mutex_lock(&lock)` and `pthread_mutex_unlock(&lock)`, so only one thread enters the critical section at a time.

### 2.2 — Semaphore as a lock

**File:** `sem.c`

```bash
gcc -Wall -pthread -o sem sem.c
./sem
./sem
./sem
```

**Recorded result:** All three runs printed `counter = 2000000 (expected 2000000)` with `sem_init(&s, 0, 1)`.

With the initial value **1**, `sem_wait(&s)` allows one thread to enter and decreases the value to 0. The other thread waits until `sem_post(&s)` makes a unit available again.

### 2.3 — Price of the lock

**Commands:**

```bash
time ./race
time ./mutex
time ./sem
```

**Recorded measurements from the Word report:**

| Program | Protection | Result | Elapsed time |
| --- | --- | --- | ---: |
| `race` | None | Incorrect (1,866,315) | 0.013 s |
| `mutex` | Mutex | 2,000,000 | 0.105 s |
| `sem` | Semaphore, initial value 1 | 2,000,000 | 0.113 s |

Times vary between runs; these numbers are from one measurement in the supplied report.

### Observe: Do mutex and semaphore always return 2,000,000?

**Yes.** In the three recorded runs of each program, both synchronization methods produced exactly 2,000,000. Both prevent simultaneous updates to the shared counter.

### Observe: What happens with `sem_init(&s, 0, 2)`?

The semaphore allows **two threads** through `sem_wait` at the same time. Since this program has two threads, both can modify the counter simultaneously and the race returns.

**Recorded result:** `counter = 1961085 (expected 2000000)`.

After this test, `sem.c` was restored to `sem_init(&s, 0, 1)`.

### Observe: Why is the locked version slower?

A mutex or semaphore adds synchronization work on every increment. Threads may also need to wait, which introduces overhead. The protected increment operations can no longer run simultaneously. This extra work makes the correct versions slower than the unprotected race in the measured runs.

## Part 3 — A semaphore as a signal

**File:** `order.c`

**Build and run:**

```bash
gcc -Wall -pthread -o order order.c
./order
```

**Expected and recorded output with `sem_init(&ready, 0, 0)`:**

```text
B: waiting for the message...
A: message written
B: I read "hello"
```

Thread B is created first but must wait at `sem_wait(&ready)` until thread A writes the message and calls `sem_post(&ready)`.

### Observe: What does B read if the initial value changes to 1, and why?

**Recorded output:**

```text
B: waiting for the message...
B: I read ""
A: message written
```

With an initial value of 1, B does not block. It reads the global message array before A writes `"hello"`, so it reads an empty string. After the experiment, the initial value was restored to **0**.

### Observe: What does a semaphore initialized to 0 mean?

It means the event has **not happened yet**. A thread calling `sem_wait` must wait until another thread signals the event using `sem_post`.

## Part 4 — A semaphore as a counter: the parking lot

**File:** `parking.c`

**Build and run:**

```bash
gcc -Wall -pthread -o parking parking.c
./parking
```

With `#define PLACES 3` and `#define CARS 6`, the semaphore begins at 3 and represents three available parking places. Each car takes a place with `sem_wait(&places)` and releases it with `sem_post(&places)`.

**Recorded observation:** Cars entered and left in varying order, but `inside` never exceeded **3**. The value returned to 0 after all cars had left.

### Observe: What is the maximum `inside` value, and what guarantees it?

The maximum recorded value is **3**. The call `sem_wait(&places)`, together with `sem_init(&places, 0, PLACES)` where `PLACES` is 3, prevents a fourth car from entering until another car releases a place with `sem_post(&places)`. The mutex protects updates and printing of `inside`, not the parking capacity.

### Observe: What happens if `PLACES` is set to 1?

The semaphore becomes a **binary semaphore**: only one car can enter at a time. In the recorded test, `inside` alternated between **1** and **0**, and cars entered sequentially. Afterward, `PLACES` was restored to **3**.

## Optional — No lock at all: atomic operation

**File:** `atomic.c`

```bash
gcc -Wall -pthread -o atomic atomic.c
time ./atomic
```

`atomic_long counter` and `atomic_fetch_add(&counter, 1)` ensure that each increment is indivisible, even though there is no explicit mutex or semaphore.

**Recorded result:** `counter = 2000000 (expected 2000000)`, with elapsed time **0.032 s**.

**Comparison run:**

```bash
time ./race
time ./mutex
time ./sem
time ./atomic
```

| Program | Elapsed time in comparison run | Result |
| --- | ---: | --- |
| `race` | 0.015 s | Incorrect (1,063,800) |
| `mutex` | 0.078 s | Correct |
| `sem` | 0.062 s | Correct |
| `atomic` | 0.040 s | Correct |

In this comparison, `atomic` was faster than both lock-based implementations but slower than the incorrect, unprotected version. Atomic operations are appropriate for a simple shared counter; a lock is still useful for coordinating a larger critical section involving several operations.

## Conclusion

Part 1 demonstrated that `counter++` is not an indivisible operation, so two threads can overwrite one another's updates and produce different incorrect results. In Part 2, both a mutex and a semaphore initialized to 1 protected the counter and consistently produced 2,000,000, although synchronization added execution overhead. Changing the semaphore's initial value to 2 allowed the race to reappear.

Part 3 showed how a semaphore initialized to 0 enforces ordering: the reader waits until the writer signals that the message is ready. Part 4 used a semaphore initialized to 3 to limit access to three parking spaces; changing it to 1 allowed only one car at a time. Finally, the optional atomic counter produced the correct result without an explicit lock and was faster than the lock-based versions in the recorded comparison.

The laboratory demonstrated that **the semaphore's initial value determines its use as a lock, signal, or resource counter**, and that synchronization is needed to guarantee correct results when threads share data.
