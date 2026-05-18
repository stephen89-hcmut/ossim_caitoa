# QUESTIONS AND DISCUSSION (SIMPLE OS)

This document contains discussion questions related to the implementation and architecture of the Simple OS assignment.

---

# Question 1

## Considering the impactness of MLQ detailed policies, what is the benefit of each policy?

Multi-Level Queue (MLQ) scheduling separates processes into different priority queues, where each queue can use a distinct scheduling policy. The benefits of each policy are:

### High-priority queues

Usually use:

- Round Robin (RR)
- Small time quantum

Benefits:

- Fast response time
- Better interactivity
- Suitable for interactive or I/O-bound processes
- Prevents UI/system lag

### Medium-priority queues

Usually balance between:

- Throughput
- Fairness

Benefits:

- Maintain stable CPU utilization
- Reduce starvation risk
- Provide balanced execution for normal workloads

### Low-priority queues

Usually use:

- FCFS (First Come First Serve)
- Longer time quantum

Benefits:

- Reduce context switching overhead
- Suitable for CPU-bound background tasks
- Improve throughput efficiency

### MLQ starvation prevention policy

The assignment also introduces starvation prevention mechanisms such as slot redistribution or priority balancing.

Benefits:

- Prevent low-priority processes from waiting indefinitely
- Improve fairness
- Increase system stability under heavy workloads

---

# Question 2

## What is the primary motivation for combining segmentation with paging in memory management? How does this hybrid approach address limitations inherent in using either technique alone?

The primary motivation for combining segmentation with paging is to leverage the advantages of both techniques while minimizing their weaknesses.

---

## Segmentation advantages

Segmentation:

- Reflects logical program structure
- Separates:
  - code
  - stack
  - heap
  - data
- Provides logical protection and sharing

However, segmentation alone suffers from:

- External fragmentation
- Difficult memory compaction
- Complex allocation management

---

## Paging advantages

Paging:

- Eliminates external fragmentation
- Simplifies physical memory allocation
- Supports virtual memory efficiently

However, paging alone:

- Does not reflect logical program structure
- Causes internal fragmentation
- Makes fine-grained protection harder

---

## Hybrid approach benefits

Combining segmentation with paging provides:

### Logical organization

Segmentation preserves logical separation between memory regions.

### Efficient physical allocation

Paging manages physical memory in fixed-size pages.

### Better protection

Each segment can have:

- different access permissions
- independent growth limits
- isolated address spaces

### Reduced fragmentation

Paging eliminates external fragmentation inside segments.

### Scalability

The system becomes more suitable for:

- large address spaces
- virtual memory
- multi-process operating systems

---

# Question 3

## What are the benefits of extending hierarchical paging to N level?

Extending hierarchical paging to N-level paging provides several important benefits for modern operating systems.

---

## Reduced memory overhead

Single-level page tables become extremely large in 64-bit address spaces.

Hierarchical paging allocates lower-level tables only when needed.

Benefits:

- Save memory
- Avoid allocating unused page tables
- Improve scalability

---

## Support for very large virtual address spaces

Modern systems use:

- 48-bit
- 57-bit
- or larger virtual addresses

N-level paging allows the operating system to:

- manage huge address spaces
- organize address translation efficiently

Example in the assignment:

```text
PGD -> P4D -> PUD -> PMD -> PT
```

---

## Sparse memory optimization

Most processes do not use the entire virtual address space.

Hierarchical paging allocates page tables only for used regions.

Benefits:

- Lower memory waste
- Better efficiency

---

## Better modularity and maintainability

Each level handles a smaller portion of the address translation.

Benefits:

- Cleaner design
- Easier debugging
- Easier extension to future architectures

---

## Compatibility with modern CPU architectures

Modern Linux and x86_64 architectures already use multi-level paging.

Benefits:

- Similarity to real operating system design
- Better understanding of practical OS memory management

---

# Question 4

## What happens if the synchronization is not handled in your Simple OS? Illustrate the problem of your simple OS (assignment outputs) by example if you have any in the added kernel memory operations.

If synchronization is not handled properly in the Simple OS, multiple CPUs or threads may concurrently access shared kernel resources, causing inconsistent system states and unpredictable behavior.

---

## Possible problems

### Race Condition

Two or more CPUs modify shared structures simultaneously.

Example:

- `free_fp_list`
- `mlq_ready_queue`
- page tables
- global timer variables

Possible consequences:

- duplicated frame allocation
- corrupted queue state
- invalid page mappings

---

### Deadlock

Improper mutex usage may cause CPUs to wait forever.

Example:

- CPU 0 locks memory manager
- CPU 1 locks scheduler
- both wait for each other

Consequences:

- system freeze
- terminal hang
- infinite waiting

---

### Memory corruption

Without synchronization:

- two processes may receive the same physical frame
- page tables may be overwritten
- invalid address translation may occur

Consequences:

- segmentation faults
- incorrect data reads/writes
- random crashes

---

## Example scenario in Simple OS

Suppose two CPUs simultaneously allocate memory pages:

```text
CPU 0:
alloc_page_range ()

CPU 1:
alloc_page_range ()
```

Without `pthread_mutex_lock ()`, both CPUs may:

- read the same free frame
- assign identical frame IDs
- remove the same node from `free_fp_list`

Result:

```text
Process A -> frame 15
Process B -> frame 15
```

This creates memory aliasing and corrupts virtual memory isolation.

---

## Possible assignment output symptoms

Example abnormal outputs:

```text
Segmentation fault
double free detected
invalid page table entry
queue corruption
system halted
```

or:

```text
Terminal frozen while running MLQ + Paging
```

---

## Importance of synchronization

Synchronization ensures:

- consistency
- correctness
- safe concurrent execution
- reliable virtual memory management

Therefore, all shared kernel resources in the assignment must be protected using mutex mechanisms such as:

```c
pthread_mutex_lock (&mem_lock);
pthread_mutex_unlock (&mem_lock);
```

---