# Laboratory Work No. 12 — Producer, Consumer and the Other Tools

**Course:** Operating Systems (FCIM / FAF / UTM, 2026–2027)  
**Student:** Botezatu Gabriela, FAF-243  
**Environment:** Ubuntu 26.04 LTS (VirtualBox), GCC 15.2.0, POSIX threads

This file contains the answers to all **12 Observe questions** from Parts 1–4 of Lab 12. Terminal screenshots, tests, and fuller explanations are included in `Lab12_BotezatuGabriela_FAF243.docx`.

## Part 1 — Producer and Consumer

### Observe 1 — The producer is fast, yet "in buffer" never goes above 4. Which call stops it?

**`sem_wait(&empty)` stops the producer.** The counting semaphore `empty` begins at 4 and decreases after each reserved slot. Once four items have filled the buffer, `empty` becomes 0 and the next `sem_wait(&empty)` blocks. The producer resumes only after the consumer removes an item and calls `sem_post(&empty)`.

**Command tested:** `gcc -Wall -pthread -o prodcons prodcons.c && ./prodcons`

**Observed result:** The buffer count reached 4 but never exceeded it. The consumer removed all eight items in order, leaving the buffer empty.

### Observe 2 — Who waits at the start: the consumer or the producer? Which call makes it wait?

**The consumer waits first**, at `sem_wait(&full)`. The semaphore `full` starts at 0 because no items are available. After the producer places the first item and calls `sem_post(&full)`, the consumer can continue. The producer can initially fill up to four free slots because `empty` starts at 4.

### Observe 3 — What happens if the producer locks the mutex before `sem_wait(&empty)`?

**The program deadlocks when the buffer fills.** The producer holds `lock` while waiting for an empty slot. The consumer needs that same mutex to remove an item and call `sem_post(&empty)`. Neither thread can proceed.

**Change tested:**

```c
pthread_mutex_lock(&lock);
sem_wait(&empty);
```

**Observed result:** The program printed `put 1` through `put 4`, then stopped making progress. It was interrupted using Ctrl-C. The original order (`sem_wait` before locking the mutex) was restored.

## Part 2 — The Condition Variable

### Observe 1 — Which variable remembers that the message is ready?

The regular integer **`ready`** stores the state. The mutex `lock` protects this variable, while the condition variable `changed` lets the reader sleep and be awakened. Unlike the semaphore used in Lab 11, a condition variable **does not remember signals** sent when no thread is waiting.

**Command tested:** `gcc -Wall -pthread -o condvar condvar.c && ./condvar`

**Observed output:**

```text
B: not ready, I sleep
A: message written
B: I read "hello"
```

### Observe 2 — Does replacing `while (!ready)` with `if (!ready)` still work here?

**Yes, in this test the output stayed the same.** There is one reader and one writer, and the writer signals after setting `ready = 1`. The test therefore still printed the message in the correct order. After testing, `while (!ready)` was restored.

### Observe 3 — Why should we use `while` instead of `if`?

**Waking up does not guarantee that `ready` is true.** A condition-variable wait can return because of a *spurious wakeup*. Also, another thread might change the shared state before the awakened thread reacquires the mutex. With `while`, the condition is checked again; with `if`, the thread might continue incorrectly while `ready == 0`.

**Relevant code:**

```c
while (!ready) {
    pthread_cond_wait(&changed, &lock);
}
```

## Part 3 — Readers and Writers

### Observe 1 — Which two lines let three readers enter together while a writer stays alone?

These two lines in `reader()` control the shared room:

```c
if (++readers == 1) sem_wait(&room);
if (--readers == 0) sem_post(&room);
```

The **first reader** locks `room` against writers, and the **last reader** releases it. Readers arriving between these events do not acquire `room`, so multiple readers can read simultaneously. Each writer must acquire `room` by itself.

**Command tested:** `gcc -Wall -pthread -o readers readers.c && ./readers`

**Observed result:** Three readers read together (with `readers inside` increasing to 3), whereas the two writers entered separately and updated the shared value by 10 each. The scheduling order can vary across runs.

### Observe 2 — Why does the counter `readers` need its own lock (`rlock`)?

**`rlock` prevents a race condition** when multiple readers update `readers` concurrently. The expressions `++readers` and `--readers` are not indivisible operations. Without the lock, updates can be lost, the first/last-reader tests can be wrong, and threads might wait forever or allow writers in incorrectly. `rlock` protects both the counter update and its associated test.

### Observe 3 — What happens to writer 2 if readers continuously arrive?

**Writer 2 may suffer starvation.** As long as at least one reader remains inside, `readers` never reaches 0, so `room` is not released for a waiting writer. Other threads still make progress, but that writer can wait indefinitely. This reader-preference algorithm can be improved by giving waiting writers priority or introducing a fair turnstile.

## Part 4 — Build a Deadlock, Then Remove It

### Observe 1 — In the stuck run, which thread holds which mutex and waits for which one? Draw the circle.

**Thread `one` holds A and waits for B; thread `two` holds B and waits for A.** This is a circular wait, so neither can finish and release its first mutex.

```text
one holds A  -- waits for B -->  two holds B
     ^                              |
     |--------- waits for A --------|
```

**Command tested:** `gcc -Wall -pthread -o deadlock deadlock.c && timeout 5 ./deadlock; echo "exit code: $?"`

**Observed result:** Both threads printed that they held one mutex and wanted the other; the program then stopped progressing. The timeout ended the run with **exit code 124**.

### Observe 2 — If both `sleep(1)` calls are removed from the original version, does it still always hang? Why is that worse?

**No. In the ten runs documented in the report, 7 finished normally and 3 deadlocked** (runs 1, 5, and 9). Without the sleeps, the result depends on thread scheduling: sometimes one thread acquires and releases both mutexes before the other thread enters, but sometimes each holds one mutex and waits for the other.

This is **worse because the bug becomes intermittent** and harder to reproduce during testing, even though deadlock remains possible.

**Observed exit codes:** `0` for a completed run; `124` when `timeout` stopped a deadlocked run.

### Observe 3 — Why can taking the locks in the same order never deadlock?

**When every thread acquires A before B, circular waiting between these two locks is impossible**, because no thread can hold B while waiting to acquire A.

**Fix tested:** Both threads lock A first and B second, then unlock in the reverse order.

**Observed result:** Both threads printed `I have both`, the program printed `done`, and the exit code was **0**.

## Conclusion

Lab 12 combined **counting semaphores, mutexes, and condition variables** in producer–consumer and readers–writers problems. The tests showed how semaphores enforce resource limits, how a mutex protects shared state, why condition waits must re-check their conditions, and how incorrect lock ordering creates deadlock. The final fixed program demonstrates that using one consistent lock order removes the circular-wait problem.
