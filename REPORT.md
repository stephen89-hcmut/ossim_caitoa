# BÁO CÁO BÀI TẬP LỚN
# HỆ ĐIỀU HÀNH (CO2018)

ĐẠI HỌC QUỐC GIA THÀNH PHỐ HỒ CHÍ MINH  
TRƯỜNG ĐẠI HỌC BÁCH KHOA  
KHOA KHOA HỌC VÀ KỸ THUẬT MÁY TÍNH

**Chủ đề:** Simple Operating System  
**Giảng viên hướng dẫn:** Nguyễn Tuấn Huy  
**Nhóm sinh viên thực hiện:** VLVH-1 - Lớp L03  
**Thời gian:** 05/2026

---

## Danh sách thành viên và phân công công việc

| STT | Họ và tên | MSSV | Phân công | Tỉ lệ | 
| --- | --- | --- | --- | --- |
| 1 | Nguyễn Thanh Tùng | 2449106 | Tổng hợp báo cáo, Scheduler | - |
| 2 | Nguyễn Hoàng | 2433007 | Memory Management | - |
| 3 | Dương Quang Minh | 2412034 | System Call, kiểm thử | - |

**Bảng 1:** Danh sách thành viên và phân công công việc.

---

## Danh sách ký hiệu

- TS: Time slot
- PTE: Page Table Entry
- PGD/P4D/PUD/PMD/PT: Các tầng bảng trang 5 cấp
- PGN/FPN: Page Number / Frame Page Number
- VMA: Virtual Memory Area

## Danh sách từ viết tắt

- MLQ: Multi-Level Queue
- MM64: Paging 5 cấp 64-bit
- PCB: Process Control Block
- TLB: Translation Lookaside Buffer
- RAM/SWAP: Bộ nhớ chính/Bộ nhớ hoán đổi

---

## Mục lục

1. Tổng quan về bài tập lớn  
1.1. Mô tả chung  
1.2. Mục tiêu  
1.3. Phương pháp thực hiện  
2. Cấu trúc hệ thống và cách hiện thực  
2.1. Scheduler  
2.1.1. Vai trò  
2.1.2. Cấu trúc  
2.1.3. Hiện thực  
2.1.4. Kết quả kiểm thử  
2.1.5. Trả lời câu hỏi  
2.2. Memory Management  
2.2.1. Cấu trúc  
2.2.2. Hiện thực  
2.2.3. Kết quả kiểm thử  
2.2.4. Trả lời câu hỏi  
2.3. System Call  
2.3.1. Cấu trúc  
2.3.2. Hiện thực  
2.3.3. Kết quả kiểm thử  
2.3.4. Trả lời câu hỏi  
2.4. Put it all together  
Tài liệu tham khảo

---

## Danh sách hình ảnh

1. Sơ đồ hệ thống MLQ và cơ chế cấp slot
2. Biểu đồ Gantt của các process trong test sched
3. Biểu đồ Gantt của các process trong test sched_0
4. Biểu đồ Gantt của các process trong test sched_1
5. Sơ đồ phân cấp địa chỉ ảo 5 tầng (PGD → P4D → PUD → PMD → PT)
6. Luồng xử lý page fault và swap trong MM64
7. Sơ đồ giao tiếp System Call và kernel
8. Sơ đồ tích hợp các module Scheduler - Memory - Syscall

## Danh sách bảng biểu

1. Danh sách thành viên và phân công công việc
2. Thống kê MM64 giữa cấu hình 1K và 4K

---

# 1 Tổng quan về bài tập lớn

## 1.1 Mô tả chung

Mục tiêu của bài tập lớn là mô phỏng một hệ điều hành đơn giản với ba thành phần lõi: bộ lập lịch MLQ, quản lý bộ nhớ phân trang 5 cấp (MM64), và cơ chế System Call. Toàn bộ triển khai được hiện thực trong mã nguồn C tại thư mục [src/](src/) và đối chứng bằng các file kết quả trong [output/](output/).

## 1.2 Mục tiêu

1. **Scheduler:** Triển khai MLQ, quản lý tiến trình, đảm bảo công bằng giữa ưu tiên cao và thấp.
2. **Memory Management:** Quản lý bộ nhớ ảo bằng phân trang 5 cấp, hỗ trợ swap và thống kê truy cập.
3. **System Call:** Thiết kế giao diện syscall tối thiểu và an toàn cho thao tác bộ nhớ.
4. **Testing:** Chạy bộ test theo [run.sh](run.sh) để kiểm chứng tính đúng đắn.

## 1.3 Phương pháp thực hiện

- **Phát triển:** C, build bằng Makefile.
- **Kiểm thử:** Chạy batch bằng [run.sh](run.sh).
- **Phân tích:** Dựa trên log trong [output/](output/) và thống kê MM64.

---

# 2 Cấu trúc hệ thống và cách hiện thực

## 2.1 Scheduler

### 2.1.1 Vai trò

Scheduler quyết định thứ tự cấp CPU cho các tiến trình, cân bằng giữa tính đáp ứng và thông lượng khi có nhiều CPU ảo và nhiều mức ưu tiên.

### 2.1.2 Cấu trúc

Hệ thống dùng MLQ với `MAX_PRIO = 140` trong [include/os-cfg.h](include/os-cfg.h). Mỗi mức ưu tiên có hàng đợi riêng `mlq_ready_queue`, và được cấp số slot theo công thức `MAX_PRIO - prio` trong [src/sched.c](src/sched.c).

### 2.1.3 Hiện thực

- Hàng đợi FIFO được cài đặt qua `enqueue()` và `dequeue()` trong [src/queue.c](src/queue.c).
- Lựa chọn tiến trình theo MLQ trong `get_mlq_proc()` tại [src/sched.c](src/sched.c), có cơ chế reset slot khi tất cả mức ưu tiên đã cạn.
- Đồng bộ truy cập qua `queue_lock` trong [src/sched.c](src/sched.c).

1. Hàm `enqueue()` trong [src/queue.c](src/queue.c):

```c
void
enqueue (struct queue_t *q, struct pcb_t *proc)
{
    /* TODO: put a new process to queue [q] */

    if (q == NULL || proc == NULL || q->size >= MAX_QUEUE_SIZE)
        {
            return;
        }

    q->proc[q->size] = proc;
    q->size++;
}
```

Giải thích: Hàng đợi được lưu dưới dạng mảng `proc[]`, phần tử mới được thêm vào cuối mảng, tăng `size` lên 1 nếu còn chỗ trống.

2. Hàm `dequeue()` trong [src/queue.c](src/queue.c):

```c
struct pcb_t *
dequeue (struct queue_t *q)
{
    struct pcb_t *proc;
    int i;

    /* TODO: return a pcb whose prioprity is the highest
     * in the queue [q] and remember to remove it from q
     * */

    if (q == NULL || empty (q))
        {
            return NULL;
        }

    proc = q->proc[0];
    for (i = 0; i < q->size - 1; i++)
        {
            q->proc[i] = q->proc[i + 1];
        }

    q->proc[q->size - 1] = NULL;
    q->size--;

    return proc;
}
```

Giải thích: Phần tử đầu hàng đợi được lấy ra, các phần tử còn lại dồn lên một vị trí, giảm `size` để giữ FIFO.

3. Hàm `get_mlq_proc()` trong [src/sched.c](src/sched.c):

```c
struct pcb_t *
get_mlq_proc (void)
{
    struct pcb_t *proc;
    int i;

    /* TODO: get a process from PRIORITY [ready_queue].
     *      It worth to protect by a mechanism.
     * */

    pthread_mutex_lock (&queue_lock);

    if (queue_empty ())
        {
            pthread_mutex_unlock (&queue_lock);
            return NULL;
        }

    if (all_active_slots_empty ())
        {
            reset_all_slots ();
        }

    proc = NULL;
    for (i = 0; i < MAX_PRIO; i++)
        {
            if (!empty (&mlq_ready_queue[i]) && slot[i] > 0)
                {
                    proc = dequeue (&mlq_ready_queue[i]);
                    if (proc != NULL)
                        {
                            slot[i]--;
                            break;
                        }
                }
        }

    if (proc == NULL)
        {
            for (i = 0; i < MAX_PRIO; i++)
                {
                    if (!empty (&mlq_ready_queue[i]))
                        {
                            proc = dequeue (&mlq_ready_queue[i]);
                            if (proc != NULL)
                                {
                                    break;
                                }
                        }
                }
        }

    if (proc != NULL)
        {
            running_list_add (&running_list, proc);
        }

    pthread_mutex_unlock (&queue_lock);

    return proc;
}
```

Giải thích: Hệ thống duyệt từ ưu tiên cao xuống thấp, chỉ chọn hàng đợi còn slot. Nếu mọi slot đều hết, reset lại toàn bộ slot rồi chọn tiếp.

4. Hàm `put_mlq_proc()` trong [src/sched.c](src/sched.c):

```c
void
put_mlq_proc (struct pcb_t *proc)
{
    /* TODO: put running proc to running_list
     *       It worth to protect by a mechanism.
     * */

    if (proc == NULL)
        {
            return;
        }

    attach_kernel_queues (proc);

    pthread_mutex_lock (&queue_lock);
    running_list_remove (&running_list, proc);

    if (!valid_prio ((int) proc->prio))
        {
            proc->prio = MAX_PRIO - 1;
        }

    enqueue (&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock (&queue_lock);
}
```

Giải thích: Tiến trình được đưa khỏi danh sách đang chạy và quay lại hàng đợi MLQ theo mức ưu tiên, đảm bảo truy cập đồng bộ bằng mutex.

5. Hàm `cpu_routine()` trong [src/os.c](src/os.c):

```c
static void * cpu_routine(void * args) {
    struct timer_id_t * timer_id = ((struct cpu_args*)args)->timer_id;
    int id = ((struct cpu_args*)args)->id;
    /* Check for new process in ready queue */
    int time_left = 0;
    struct pcb_t * proc = NULL;
    while (1) {
        /* Check the status of current process */
        if (proc == NULL) {
            /* No process is running, the we load new process from
             * ready queue */
            proc = get_proc();
            if (proc == NULL) {
               next_slot(timer_id);
               continue; /* First load failed. skip dummy load */
            }
        }else if (proc->pc == proc->code->size) {
            /* The porcess has finish it job */
            flockfile(stdout);
            printf("\tCPU %d: Processed %2d has finished\n",
                id ,proc->pid);
            funlockfile(stdout);
            free(proc);
            proc = get_proc();
            time_left = 0;
        }else if (time_left == 0) {
            /* The process has done its job in current time slot */
            flockfile(stdout);
            printf("\tCPU %d: Put process %2d to run queue\n",
                id, proc->pid);
            funlockfile(stdout);
            put_proc(proc);
            proc = get_proc();
        }

        /* Recheck process status after loading new process */
        if (proc == NULL && done) {
            /* No process to run, exit */
            printf("\tCPU %d stopped\n", id);
            break;
        }else if (proc == NULL) {
            /* There may be new processes to run in
             * next time slots, just skip current slot */
            next_slot(timer_id);
            continue;
        }else if (time_left == 0) {
            flockfile(stdout);
            printf("\tCPU %d: Dispatched process %2d\n",
                id, proc->pid);
            funlockfile(stdout);
            time_left = time_slot;
        }

        /* Run current process */
        run(proc);
        time_left--;
        next_slot(timer_id);
    }
    flockfile(stdout);
    detach_event(timer_id);
    funlockfile(stdout);
    pthread_exit(NULL);
}
```

Giải thích: CPU lấy tiến trình từ scheduler, thực thi theo time slot, đẩy lại hàng đợi khi hết quantum và dừng khi không còn tiến trình.

6. Mã nguồn đầy đủ `queue.c` trong [src/queue.c](src/queue.c):

```c
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int
empty (struct queue_t *q)
{
        if (q == NULL)
                {
                        return 1;
                }

        return (q->size == 0);
}

void
enqueue (struct queue_t *q, struct pcb_t *proc)
{
        /* TODO: put a new process to queue [q] */

        if (q == NULL || proc == NULL || q->size >= MAX_QUEUE_SIZE)
                {
                        return;
                }

        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t *
dequeue (struct queue_t *q)
{
        struct pcb_t *proc;
        int i;

        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */

        if (q == NULL || empty (q))
                {
                        return NULL;
                }

        proc = q->proc[0];
        for (i = 0; i < q->size - 1; i++)
                {
                        q->proc[i] = q->proc[i + 1];
                }

        q->proc[q->size - 1] = NULL;
        q->size--;

        return proc;
}
```

Giải thích: Module queue cung cấp FIFO cho scheduler, với `enqueue()` thêm cuối mảng và `dequeue()` lấy đầu mảng.

7. Mã nguồn đầy đủ `sched.c` trong [src/sched.c](src/sched.c):

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "queue.h"
#include "sched.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static struct queue_t ready_queue;
static struct queue_t run_queue;
static struct running_list_t running_list;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

static void
running_list_init (struct running_list_t *list)
{
    if (list == NULL)
        {
            return;
        }

    list->head = NULL;
    list->tail = NULL;
}

static void
running_list_add (struct running_list_t *list, struct pcb_t *proc)
{
    struct running_node_t *node;

    if (list == NULL || proc == NULL)
        {
            return;
        }

    node = (struct running_node_t *) malloc (sizeof (struct running_node_t));
    if (node == NULL)
        {
            return;
        }

    node->proc = proc;
    node->next = NULL;
    if (list->tail == NULL)
        {
            list->head = node;
            list->tail = node;
            return;
        }

    list->tail->next = node;
    list->tail = node;
}

static void
running_list_remove (struct running_list_t *list, struct pcb_t *proc)
{
    struct running_node_t *prev;
    struct running_node_t *curr;

    if (list == NULL || proc == NULL)
        {
            return;
        }

    prev = NULL;
    curr = list->head;
    while (curr != NULL)
        {
            if (curr->proc == proc)
                {
                    if (prev == NULL)
                        {
                            list->head = curr->next;
                        }
                    else
                        {
                            prev->next = curr->next;
                        }

                    if (list->tail == curr)
                        {
                            list->tail = prev;
                        }

                    free (curr);
                    return;
                }

            prev = curr;
            curr = curr->next;
        }
}

#ifdef MLQ_SCHED
static int
valid_prio (int prio)
{
    return (prio >= 0 && prio < MAX_PRIO);
}

static int
all_active_slots_empty (void)
{
    int i;

    for (i = 0; i < MAX_PRIO; i++)
        {
            if (!empty (&mlq_ready_queue[i]) && slot[i] > 0)
                {
                    return 0;
                }
        }

    return 1;
}

static void
reset_all_slots (void)
{
    int i;

    for (i = 0; i < MAX_PRIO; i++)
        {
            slot[i] = MAX_PRIO - i;
            if (slot[i] < 1)
                {
                    slot[i] = 1;
                }
        }
}

static struct pcb_t *
find_proc_by_pid_in_queue (struct queue_t *q, uint32_t pid)
{
    int i;

    if (q == NULL)
        {
            return NULL;
        }

    for (i = 0; i < q->size; i++)
        {
            if (q->proc[i] != NULL && q->proc[i]->pid == pid)
                {
                    return q->proc[i];
                }
        }

    return NULL;
}

static struct pcb_t *
find_proc_by_pid_in_running_list (struct running_list_t *list, uint32_t pid)
{
    struct running_node_t *node;

    if (list == NULL)
        {
            return NULL;
        }

    for (node = list->head; node != NULL; node = node->next)
        {
            if (node->proc != NULL && node->proc->pid == pid)
                {
                    return node->proc;
                }
        }

    return NULL;
}
#endif

int
queue_empty (void)
{
#ifdef MLQ_SCHED
    int prio;

    for (prio = 0; prio < MAX_PRIO; prio++)
        {
            if (!empty (&mlq_ready_queue[prio]))
                {
                    return 0;
                }
        }

    return 1;
#else
    return (empty (&ready_queue) && empty (&run_queue));
#endif
}

void
init_scheduler (void)
{
    ready_queue.size = 0;
    run_queue.size = 0;
    running_list_init (&running_list);

#ifdef MLQ_SCHED
    int i;

    for (i = 0; i < MAX_PRIO; i++)
        {
            mlq_ready_queue[i].size = 0;
            slot[i] = MAX_PRIO - i;
            if (slot[i] < 1)
                {
                    slot[i] = 1;
                }
        }
#endif
}

#ifdef MLQ_SCHED
/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t *
get_mlq_proc (void)
{
    struct pcb_t *proc;
    int i;

    /* TODO: get a process from PRIORITY [ready_queue].
 	 *      It worth to protect by a mechanism.
 	 * */

    pthread_mutex_lock (&queue_lock);

    if (queue_empty ())
        {
            pthread_mutex_unlock (&queue_lock);
            return NULL;
        }

    if (all_active_slots_empty ())
        {
            reset_all_slots ();
        }

    proc = NULL;
    for (i = 0; i < MAX_PRIO; i++)
        {
            if (!empty (&mlq_ready_queue[i]) && slot[i] > 0)
                {
                    proc = dequeue (&mlq_ready_queue[i]);
                    if (proc != NULL)
                        {
                            slot[i]--;
                            break;
                        }
                }
        }

    if (proc == NULL)
        {
            for (i = 0; i < MAX_PRIO; i++)
                {
                    if (!empty (&mlq_ready_queue[i]))
                        {
                            proc = dequeue (&mlq_ready_queue[i]);
                            if (proc != NULL)
                                {
                                    break;
                                }
                        }
                }
        }

    if (proc != NULL)
        {
            running_list_add (&running_list, proc);
        }

    pthread_mutex_unlock (&queue_lock);

    return proc;
}

static void
attach_kernel_queues (struct pcb_t *proc)
{
    if (proc == NULL || proc->krnl == NULL)
        {
            return;
        }

    proc->krnl->ready_queue = &ready_queue;
    proc->krnl->running_list = &running_list;
    proc->krnl->mlq_ready_queue = mlq_ready_queue;
}

void
put_mlq_proc (struct pcb_t *proc)
{
    /* TODO: put running proc to running_list
 	 *       It worth to protect by a mechanism.
 	 * */

    if (proc == NULL)
        {
            return;
        }

    attach_kernel_queues (proc);

    pthread_mutex_lock (&queue_lock);
    running_list_remove (&running_list, proc);

    if (!valid_prio ((int) proc->prio))
        {
            proc->prio = MAX_PRIO - 1;
        }

    enqueue (&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock (&queue_lock);
}

void
add_mlq_proc (struct pcb_t *proc)
{
    /* TODO: put running proc to running_list
 	 *       It worth to protect by a mechanism.
 	 * */

    if (proc == NULL)
        {
            return;
        }

    attach_kernel_queues (proc);

    if (!valid_prio ((int) proc->prio))
        {
            proc->prio = MAX_PRIO - 1;
        }

    pthread_mutex_lock (&queue_lock);
    enqueue (&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock (&queue_lock);
}

struct pcb_t *
get_proc (void)
{
    /* TODO: get a process from [ready_queue].
 	 *       It worth to protect by a mechanism.
 	 * */

    return get_mlq_proc ();
}

void
put_proc (struct pcb_t *proc)
{
    /* TODO: put running proc to running_list
 	 *       It worth to protect by a mechanism.
 	 * */

    put_mlq_proc (proc);
}

void
add_proc (struct pcb_t *proc)
{
    /* TODO: put running proc to running_list
 	 *       It worth to protect by a mechanism.
 	 * */

    add_mlq_proc (proc);
}

struct pcb_t *
find_proc_by_pid (struct krnl_t *krnl, uint32_t pid)
{
    struct pcb_t *caller;
    int i;

    if (krnl == NULL)
        {
            return NULL;
        }

    pthread_mutex_lock (&queue_lock);

    caller = find_proc_by_pid_in_queue (krnl->ready_queue, pid);
    if (caller != NULL)
        {
            pthread_mutex_unlock (&queue_lock);
            return caller;
        }

    caller = find_proc_by_pid_in_running_list (krnl->running_list, pid);
    if (caller != NULL)
        {
            pthread_mutex_unlock (&queue_lock);
            return caller;
        }

    if (krnl->mlq_ready_queue != NULL)
        {
            for (i = 0; i < MAX_PRIO; i++)
                {
                    caller = find_proc_by_pid_in_queue (&krnl->mlq_ready_queue[i], pid);
                    if (caller != NULL)
                        {
                            pthread_mutex_unlock (&queue_lock);
                            return caller;
                        }
                }
        }

    pthread_mutex_unlock (&queue_lock);
    return NULL;
}

void
finish_scheduler (void)
{
}
#else
struct pcb_t *
get_proc (void)
{
    struct pcb_t *proc;

    pthread_mutex_lock (&queue_lock);
    proc = dequeue (&ready_queue);
    if (proc == NULL)
        {
            proc = dequeue (&run_queue);
        }

    if (proc != NULL)
        {
            running_list_add (&running_list, proc);
        }
    pthread_mutex_unlock (&queue_lock);

    return proc;
}

void
put_proc (struct pcb_t *proc)
{
    if (proc == NULL)
        {
            return;
        }

    proc->krnl->ready_queue = &ready_queue;
    proc->krnl->running_list = &running_list;

    pthread_mutex_lock (&queue_lock);
    running_list_remove (&running_list, proc);
    enqueue (&run_queue, proc);
    pthread_mutex_unlock (&queue_lock);
}

void
add_proc (struct pcb_t *proc)
{
    if (proc == NULL)
        {
            return;
        }

    proc->krnl->ready_queue = &ready_queue;
    proc->krnl->running_list = &running_list;

    pthread_mutex_lock (&queue_lock);
    enqueue (&ready_queue, proc);
    pthread_mutex_unlock (&queue_lock);
}

struct pcb_t *
find_proc_by_pid (struct krnl_t *krnl, uint32_t pid)
{
    struct pcb_t *caller;

    if (krnl == NULL)
        return NULL;

    pthread_mutex_lock (&queue_lock);

    caller = find_proc_by_pid_in_queue (krnl->ready_queue, pid);
    if (caller != NULL) {
        pthread_mutex_unlock (&queue_lock);
        return caller;
    }

    caller = find_proc_by_pid_in_queue (krnl->run_queue, pid);
    if (caller != NULL) {
        pthread_mutex_unlock (&queue_lock);
        return caller;
    }

    caller = find_proc_by_pid_in_running_list (krnl->running_list, pid);
    if (caller != NULL) {
        pthread_mutex_unlock (&queue_lock);
        return caller;
    }

    pthread_mutex_unlock (&queue_lock);
    return NULL;
}
#endif
```

Giải thích: `sched.c` hiện thực MLQ, quản lý running_list và cơ chế slot, đồng thời bảo vệ truy cập bằng `queue_lock`.

**Hình 1:** Sơ đồ hệ thống MLQ và cơ chế cấp slot.

### 2.1.4 Kết quả kiểm thử

**Test sched:**

```text
Time slot   0
ld_routine
    Loaded a process at input/proc/p2s, PID: 1 PRIO: 0
    CPU 0: Dispatched process  1
Time slot   2
    CPU 1: Dispatched process  2
```

**Test sched_0:**

```text
Time slot   4
    Loaded a process at input/proc/s1, PID: 3 PRIO: 0
Time slot   5
    CPU 0: Put process  2 to run queue
    CPU 0: Dispatched process  3
```

**Test sched_1:**

```text
Time slot   2
    Loaded a process at input/proc/s3, PID: 2 PRIO: 0
    CPU 0: Put process  1 to run queue
    CPU 0: Dispatched process  2
```

**Hình 2:** Biểu đồ Gantt của các process trong test sched.  
**Hình 3:** Biểu đồ Gantt của các process trong test sched_0.  
**Hình 4:** Biểu đồ Gantt của các process trong test sched_1.

### 2.1.5 Trả lời câu hỏi

**Câu hỏi:** Thuật toán MLQ trong bài này có ưu điểm gì so với các thuật toán khác?

**Trả lời:** MLQ kết hợp ưu tiên và Round Robin nên vẫn đảm bảo đáp ứng tốt cho tiến trình ưu tiên cao nhưng tránh starvation nhờ cơ chế reset slot. So với FCFS, MLQ giảm hiệu ứng đoàn xe; so với SJF/Priority thuần, MLQ giảm nguy cơ tiến trình thấp bị đói CPU; so với RR thuần, MLQ cho phép ưu tiên tương tác tốt hơn.

---

## 2.2 Memory Management

### 2.2.1 Cấu trúc

Hệ thống dùng phân trang 5 cấp theo MM64, định nghĩa trong [include/mm64.h](include/mm64.h). Các vùng nhớ ảo và region được quản lý bằng `vm_area_struct` và `vm_rg_struct` trong [src/mm-vm.c](src/mm-vm.c). Bộ nhớ vật lý và swap được quản lý bởi `memphy_struct` trong [src/mm-memphy.c](src/mm-memphy.c).

**Hình 5:** Sơ đồ phân cấp địa chỉ ảo 5 tầng (PGD → P4D → PUD → PMD → PT).  
**Hình 6:** Luồng xử lý page fault và swap trong MM64.

### 2.2.2 Hiện thực

- Ánh xạ địa chỉ 5 cấp được tách bởi `get_pd_from_address()` trong [src/mm64.c](src/mm64.c).
- Bảng trang và trạng thái trang được cập nhật qua `pte_set_fpn()` và `pte_set_swap()` trong [src/mm64.c](src/mm64.c).
- Cơ chế page fault và swap được xử lý trong `pg_getpage()` tại [src/libmem.c](src/libmem.c).
- Đồng bộ truy cập bộ nhớ vật lý bằng `memphy_lock` trong [src/mm-memphy.c](src/mm-memphy.c).

1. Hàm `get_pd_from_address()` trong [src/mm64.c](src/mm64.c):

```c
int get_pd_from_address(addr_t addr, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt)
{
    if (pgd == NULL || p4d == NULL || pud == NULL || pmd == NULL || pt == NULL)
        return -1;

	/* Extract page direactories */
	*pgd = (addr >> PAGING64_ADDR_PGD_LOBIT) & 0x1FF;
	*p4d = (addr >> PAGING64_ADDR_P4D_LOBIT) & 0x1FF;
	*pud = (addr >> PAGING64_ADDR_PUD_LOBIT) & 0x1FF;
	*pmd = (addr >> PAGING64_ADDR_PMD_LOBIT) & 0x1FF;
	*pt = (addr >> PAGING64_ADDR_PT_LOBIT) & 0x1FF;

	/* TODO: implement the page direactories mapping */

	return 0;
}
```

Giải thích: Tách địa chỉ ảo thành chỉ số PGD/P4D/PUD/PMD/PT theo cấu trúc 5 tầng.

2. Hàm `__alloc()` trong [src/libmem.c](src/libmem.c):

```c
int __alloc(struct pcb_t *caller, int vmaid, int rgid, addr_t size, addr_t *alloc_addr)
{
    /*Allocate at the toproof */
    pthread_mutex_lock(&mmvm_lock);
    struct vm_rg_struct rgnode;
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
    int inc_sz=0;

    if (cur_vma == NULL)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }

    if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
    {
        caller->krnl->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
        caller->krnl->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;

        *alloc_addr = rgnode.rg_start;

        pthread_mutex_unlock(&mmvm_lock);
        return 0;
    }

    /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

    /*Attempt to increate limit to get space */
#ifdef MM64
    inc_sz = PAGING64_PAGE_ALIGNSZ(size);
#else
    inc_sz = PAGING_PAGE_ALIGNSZ(size);
#endif
    int old_sbrk;

    old_sbrk = cur_vma->sbrk;

    /* TODO INCREASE THE LIMIT
     * SYSCALL 1 sys_memmap
     */
    struct sc_regs regs;
    regs.a1 = SYSMEM_INC_OP;
    regs.a2 = vmaid;
#ifdef MM64
    regs.a3 = inc_sz;
#else
    regs.a3 = PAGING_PAGE_ALIGNSZ(size);
#endif
    _syscall(caller->krnl, caller->pid, 17, &regs); /* SYSCALL 17 sys_memmap */

    /*Successful increase limit */
    caller->krnl->mm->symrgtbl[rgid].rg_start = old_sbrk;
    caller->krnl->mm->symrgtbl[rgid].rg_end = old_sbrk + size;

    if (inc_sz > size)
    {
        struct vm_rg_struct *remain_rg = malloc(sizeof(struct vm_rg_struct));
        if (remain_rg != NULL)
        {
            remain_rg->rg_start = old_sbrk + size;
            remain_rg->rg_end = old_sbrk + inc_sz;
            remain_rg->rg_next = NULL;
            enlist_vm_freerg_list(caller->krnl->mm, remain_rg);
        }
    }

    *alloc_addr = old_sbrk;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;

}
```

Giải thích: Hệ thống ưu tiên lấy vùng trống, nếu thiếu thì tăng giới hạn VMA bằng syscall 17 và cập nhật bảng symbol.

3. Hàm `__free()` trong [src/libmem.c](src/libmem.c):

```c
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
    pthread_mutex_lock(&mmvm_lock);

    if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }

    /* TODO: Manage the collect freed region to freerg_list */
    struct vm_rg_struct *rgnode = get_symrg_byid(caller->krnl->mm, rgid);

    if (rgnode->rg_start == 0 && rgnode->rg_end == 0)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }
    struct vm_rg_struct *freerg_node = malloc(sizeof(struct vm_rg_struct));
    freerg_node->rg_start = rgnode->rg_start;
    freerg_node->rg_end = rgnode->rg_end;
    freerg_node->rg_next = NULL;

    rgnode->rg_start = rgnode->rg_end = 0;
    rgnode->rg_next = NULL;

    /*enlist the obsoleted memory region */
    enlist_vm_freerg_list(caller->krnl->mm, freerg_node);

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
}
```

Giải thích: Vùng nhớ được giải phóng sẽ được đưa vào free list để tái sử dụng và cập nhật lại symbol table.

4. Hàm `pg_getpage()` trong [src/libmem.c](src/libmem.c):

```c
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
    uint32_t pte;
    addr_t target_fpn;

    if (mm == NULL || caller == NULL || caller->krnl == NULL || caller->krnl->mram == NULL)
        return -1;

    caller->krnl->mem_access_cnt++;

    pte = pte_get_entry(caller, pgn);

    /* TODO Initialize the target frame storing our variable */
//  addr_t tgtfpn

    if (!PAGING_PAGE_PRESENT(pte) || (pte & PAGING_PTE_SWAPPED_MASK))
    {
        caller->krnl->page_fault_cnt++;
        addr_t vicpgn = 0;
        addr_t swpfpn = 0;
//  addr_t vicfpn;
//  addr_t vicpte;
//  struct sc_regs regs;

        /* TODO: Play with your paging theory here */
        /* Find victim page */
        if (MEMPHY_get_freefp(caller->krnl->mram, &target_fpn) != 0)
        {
            uint32_t vicpte;
            addr_t vicfpn;

            caller->krnl->page_replace_cnt++;
            if (find_victim_page(caller->krnl->mm, &vicpgn) != 0)
                return -1;

            vicpte = pte_get_entry(caller, vicpgn);
            vicfpn = PAGING_FPN(vicpte);

            /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

            /* TODO copy victim frame to swap
             * SWP(vicfpn <--> swpfpn)
             * SYSCALL 1 sys_memmap
             */

            if (MEMPHY_get_freefp(caller->krnl->active_mswp, &swpfpn) != 0)
                return -1;

            if (__swap_cp_page(caller->krnl->mram, vicfpn, caller->krnl->active_mswp, swpfpn) != 0)
                return -1;

            caller->krnl->swap_out_cnt++;

            /* Update page table */
            //pte_set_swap(...);

            if (pte_set_swap(caller, vicpgn, caller->krnl->active_mswp_id, swpfpn) != 0)
                return -1;

            /* Update its online status of the target page */
            //pte_set_fpn(...);

            target_fpn = vicfpn;
        }

        if (pte & PAGING_PTE_SWAPPED_MASK)
        {
            addr_t swpoff = PAGING_SWP(pte);

            if (__swap_cp_page(caller->krnl->active_mswp, swpoff, caller->krnl->mram, target_fpn) != 0)
                return -1;

            caller->krnl->swap_in_cnt++;
        }

        if (pte_set_fpn(caller, pgn, target_fpn) != 0)
            return -1;

        enlist_pgn_node(&caller->krnl->mm->fifo_pgn, pgn);
    }

    pte = pte_get_entry(caller, pgn);
    *fpn = PAGING_FPN(pte);

    return 0;
}
```

Giải thích: Nếu trang chưa hiện diện hoặc đã swapped, hệ thống chọn frame trống hoặc thay thế trang, cập nhật PTE và thống kê swap.

5. Hàm `pg_getval()` và `pg_setval()` trong [src/libmem.c](src/libmem.c):

```c
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
    int pgn = PAGING_PGN(addr);
//int off = PAGING_OFFST(addr);
    int off = addr & (PAGING64_PAGESZ - 1);
    int fpn;

    if (pg_getpage(mm, pgn, &fpn, caller) != 0)
        return -1; /* invalid page access */

//int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

    /* TODO
     *  MEMPHY_read(caller->krnl->mram, phyaddr, data);
     *  MEMPHY READ
     *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
     */
    if (MEMPHY_read(caller->krnl->mram, (addr_t)fpn * PAGING64_PAGESZ + off, data) != 0)
        return -1;

    return 0;
}

int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
    int pgn = PAGING_PGN(addr);
//int off = PAGING_OFFST(addr);
    int off = addr & (PAGING64_PAGESZ - 1);
    int fpn;

    /* Get the page to MEMRAM, swap from MEMSWAP if needed */
    if (pg_getpage(mm, pgn, &fpn, caller) != 0)
        return -1; /* invalid page access */

    /* TODO
     *  MEMPHY_write(caller->krnl->mram, phyaddr, value);
     *  MEMPHY WRITE with SYSMEM_IO_WRITE
     * SYSCALL 17 sys_memmap
     */
    if (MEMPHY_write(caller->krnl->mram, (addr_t)fpn * PAGING64_PAGESZ + off, value) != 0)
        return -1;

    return 0;
}
```

Giải thích: Sau khi bảo đảm trang đã ở RAM, hệ thống đọc/ghi trực tiếp qua MEMPHY.

6. Mã nguồn đầy đủ `mm64.c` trong [src/mm64.c](src/mm64.c):

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* LamiaAtrium release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

/*
 * PAGING based Memory Management
 * Memory management unit mm/mm.c
 */

#include "mm64.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

static int mm64_pgn_valid (addr_t pgn)
{
    return (pgn < PAGING64_MAX_PGN);
}

static addr_t *mm64_get_pte (struct pcb_t *caller, addr_t pgn)
{
    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return NULL;

    if (caller->krnl->mm->pt == NULL || caller->krnl->mm->pgd == NULL || !mm64_pgn_valid (pgn))
        return NULL;

    caller->krnl->pgtbl_access_cnt++;

    return &caller->krnl->mm->pt[pgn];
}

#if defined(MM64)

/*
 * init_pte - Initialize PTE entry
 */
int init_pte(addr_t *pte,
                         int pre,    // present
                         addr_t fpn,    // FPN
                         int drt,    // dirty
                         int swp,    // swap
                         int swptyp, // swap type
                         addr_t swpoff) // swap offset
{
    if (pre != 0) {
        if (swp == 0) { // Non swap ~ page online
            if (fpn == 0)
                return -1;  // Invalid setting

            /* Valid setting with FPN */
            SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
            CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);
            CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

            SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);
        }
        else
        { // page swapped
            SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
            SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
            CLRBIT(*pte, PAGING_PTE_DIRTY_MASK);

            SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
            SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);
        }
    }

    return 0;
}


/*
 * get_pd_from_pagenum - Parse address to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table 
 */
int get_pd_from_address(addr_t addr, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt)
{
    if (pgd == NULL || p4d == NULL || pud == NULL || pmd == NULL || pt == NULL)
        return -1;

	/* Extract page direactories */
	*pgd = (addr >> PAGING64_ADDR_PGD_LOBIT) & 0x1FF;
	*p4d = (addr >> PAGING64_ADDR_P4D_LOBIT) & 0x1FF;
	*pud = (addr >> PAGING64_ADDR_PUD_LOBIT) & 0x1FF;
	*pmd = (addr >> PAGING64_ADDR_PMD_LOBIT) & 0x1FF;
	*pt = (addr >> PAGING64_ADDR_PT_LOBIT) & 0x1FF;

	/* TODO: implement the page direactories mapping */

	return 0;
}

/*
 * get_pd_from_pagenum - Parse page number to 5 page directory level
 * @pgn   : pagenumer
 * @pgd   : page global directory
 * @p4d   : page level directory
 * @pud   : page upper directory
 * @pmd   : page middle directory
 * @pt    : page table 
 */
int get_pd_from_pagenum(addr_t pgn, addr_t* pgd, addr_t* p4d, addr_t* pud, addr_t* pmd, addr_t* pt)
{
	/* Shift the address to get page num and perform the mapping*/
	return get_pd_from_address(pgn << PAGING64_ADDR_PT_SHIFT,
                                                 pgd,p4d,pud,pmd,pt);
}


/*
 * pte_set_swap - Set PTE entry for swapped page
 * @pte    : target page table entry (PTE)
 * @swptyp : swap type
 * @swpoff : swap offset
 */
int pte_set_swap(struct pcb_t *caller, addr_t pgn, int swptyp, addr_t swpoff)
{
//struct krnl_t *krnl = caller->krnl;

    addr_t *pte = mm64_get_pte (caller, pgn);

    if (pte == NULL)
        return -1;

    *pte = 0;
    SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
    SETBIT(*pte, PAGING_PTE_SWAPPED_MASK);
    SETVAL(*pte, swptyp, PAGING_PTE_SWPTYP_MASK, PAGING_PTE_SWPTYP_LOBIT);
    SETVAL(*pte, swpoff, PAGING_PTE_SWPOFF_MASK, PAGING_PTE_SWPOFF_LOBIT);

    if (caller->krnl != NULL && caller->krnl->mm != NULL && caller->krnl->mm->pgd != NULL && mm64_pgn_valid (pgn))
        caller->krnl->mm->pgd[pgn] = *pte;

    return 0;
}

/*
 * pte_set_fpn - Set PTE entry for on-line page
 * @pte   : target page table entry (PTE)
 * @fpn   : frame page number (FPN)
 */
int pte_set_fpn(struct pcb_t *caller, addr_t pgn, addr_t fpn)
{
//struct krnl_t *krnl = caller->krnl;

    addr_t *pte = mm64_get_pte (caller, pgn);

    if (pte == NULL)
        return -1;

    *pte = 0;
    SETBIT(*pte, PAGING_PTE_PRESENT_MASK);
    CLRBIT(*pte, PAGING_PTE_SWAPPED_MASK);

    SETVAL(*pte, fpn, PAGING_PTE_FPN_MASK, PAGING_PTE_FPN_LOBIT);

    if (caller->krnl != NULL && caller->krnl->mm != NULL && caller->krnl->mm->pgd != NULL && mm64_pgn_valid (pgn))
        caller->krnl->mm->pgd[pgn] = *pte;

    return 0;
}


/* Get PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
uint32_t pte_get_entry(struct pcb_t *caller, addr_t pgn)
{
//struct krnl_t *krnl = caller->krnl;
/* TODO Perform multi-level page mapping */
//... krnl->mm->pgd
    //... krnl->mm->pt
    //pte = &krnl->mm->pt;
    addr_t *pte = mm64_get_pte (caller, pgn);

    if (pte == NULL)
        return 0;

    return (uint32_t)(*pte);
}

/* Set PTE page table entry
 * @caller : caller
 * @pgn    : page number
 * @ret    : page table entry
 **/
int pte_set_entry(struct pcb_t *caller, addr_t pgn, uint32_t pte_val)
{
    addr_t *pte = mm64_get_pte (caller, pgn);
    addr_t pgd = 0;
    addr_t p4d = 0;
    addr_t pud = 0;
    addr_t pmd = 0;
    addr_t pt = 0;

    if (pte == NULL)
        return -1;

    *pte = pte_val;

    if (caller != NULL && caller->krnl != NULL && caller->krnl->mm != NULL)
    {
        get_pd_from_pagenum(pgn, &pgd, &p4d, &pud, &pmd, &pt);
        if (caller->krnl->mm->pgd != NULL && mm64_pgn_valid(pgn))
            caller->krnl->mm->pgd[pgn] = pte_val;
        if (caller->krnl->mm->p4d != NULL && mm64_pgn_valid(p4d))
            caller->krnl->mm->p4d[p4d] = PAGING_PTE_PRESENT_MASK;
        if (caller->krnl->mm->pud != NULL && mm64_pgn_valid(pud))
            caller->krnl->mm->pud[pud] = PAGING_PTE_PRESENT_MASK;
        if (caller->krnl->mm->pmd != NULL && mm64_pgn_valid(pmd))
            caller->krnl->mm->pmd[pmd] = PAGING_PTE_PRESENT_MASK;
        if (caller->krnl->mm->pt != NULL && mm64_pgn_valid(pt))
            caller->krnl->mm->pt[pt] = pte_val;
    }

    return 0;
}


/*
 * vmap_pgd_memset - map a range of page at aligned address
 */
int vmap_pgd_memset(struct pcb_t *caller,           // process call
                                        addr_t addr,                       // start address which is aligned to pagesz
                                        int pgnum)                      // num of mapping page
{
    //int pgit = 0;
    //uint64_t pattern = 0xdeadbeef;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return -1;

    /* TODO memset the page table with given pattern
     */

    if (caller->krnl->mm->pgd == NULL || caller->krnl->mm->p4d == NULL ||
            caller->krnl->mm->pud == NULL || caller->krnl->mm->pmd == NULL ||
            caller->krnl->mm->pt == NULL)
        return -1;

    addr_t pgn = addr >> PAGING64_ADDR_PT_SHIFT;
    if (pgnum < 0 || pgn >= PAGING64_MAX_PGN || pgn + (addr_t)pgnum > PAGING64_MAX_PGN)
        return -1;

    memset(&caller->krnl->mm->pgd[pgn], 0, (size_t)pgnum * sizeof(addr_t));
    memset(&caller->krnl->mm->p4d[pgn], 0, (size_t)pgnum * sizeof(addr_t));
    memset(&caller->krnl->mm->pud[pgn], 0, (size_t)pgnum * sizeof(addr_t));
    memset(&caller->krnl->mm->pmd[pgn], 0, (size_t)pgnum * sizeof(addr_t));
    memset(&caller->krnl->mm->pt[pgn], 0, (size_t)pgnum * sizeof(addr_t));

    return 0;
}

/*
 * vmap_page_range - map a range of page at aligned address
 */
addr_t vmap_page_range(struct pcb_t *caller,           // process call
                                        addr_t addr,                       // start address which is aligned to pagesz
                                        int pgnum,                      // num of mapping page
                                        struct framephy_struct *frames, // list of the mapped frames
                                        struct vm_rg_struct *ret_rg)    // return mapped region, the real mapped fp
{                                                   // no guarantee all given pages are mapped
//struct framephy_struct *fpit;
//int pgit = 0;
//addr_t pgn;

    /* TODO: update the rg_end and rg_start of ret_rg 
    //ret_rg->rg_end =  ....
    //ret_rg->rg_start = ...
    //ret_rg->vmaid = ...
    */

    struct framephy_struct *fpit = frames;
    int pgit;
    addr_t pgn = addr >> PAGING64_ADDR_PT_SHIFT;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return 0;

    if (ret_rg != NULL)
    {
        ret_rg->rg_start = addr;
        ret_rg->rg_end = addr + ((addr_t)pgnum * PAGING64_PAGESZ);
    }

    /* TODO map range of frame to address space
     *      [addr to addr + pgnum*PAGING_PAGESZ
     *      in page table caller->krnl->mm->pgd,
     *                    caller->krnl->mm->pud...
     *                    ...
     */

    for (pgit = 0; pgit < pgnum; pgit++)
    {
        if (pgn + pgit >= PAGING64_MAX_PGN)
            break;

        if (fpit != NULL)
        {
            addr_t pgd = 0;
            addr_t p4d = 0;
            addr_t pud = 0;
            addr_t pmd = 0;
            addr_t pt = 0;

            get_pd_from_pagenum(pgn + pgit, &pgd, &p4d, &pud, &pmd, &pt);
            if (caller->krnl->mm->pgd != NULL && mm64_pgn_valid(pgn + pgit))
                caller->krnl->mm->pgd[pgn + pgit] = PAGING_PTE_PRESENT_MASK;
            if (caller->krnl->mm->p4d != NULL && mm64_pgn_valid(p4d))
                caller->krnl->mm->p4d[p4d] = PAGING_PTE_PRESENT_MASK;
            if (caller->krnl->mm->pud != NULL && mm64_pgn_valid(pud))
                caller->krnl->mm->pud[pud] = PAGING_PTE_PRESENT_MASK;
            if (caller->krnl->mm->pmd != NULL && mm64_pgn_valid(pmd))
                caller->krnl->mm->pmd[pmd] = PAGING_PTE_PRESENT_MASK;
            if (caller->krnl->mm->pt != NULL && mm64_pgn_valid(pt))
                caller->krnl->mm->pt[pt] = PAGING_PTE_PRESENT_MASK;

            if (pte_set_fpn(caller, pgn + pgit, fpit->fpn) != 0)
                return 0;
            enlist_pgn_node(&caller->krnl->mm->fifo_pgn, pgn + pgit);
            fpit = fpit->fp_next;
        }
        else
        {
            if (pte_set_entry(caller, pgn + pgit, 0) != 0)
                return 0;
        }
    }

    /* Tracking for later page replacement activities (if needed)
     * Enqueue new usage page */
    //enlist_pgn_node(&caller->krnl->mm->fifo_pgn, pgn64 + pgit);

    return addr;
}

/*
 * alloc_pages_range - allocate req_pgnum of frame in ram
 * @caller    : caller
 * @req_pgnum : request page num
 * @frm_lst   : frame list
 */

addr_t alloc_pages_range(struct pcb_t *caller, int req_pgnum, struct framephy_struct **frm_lst)
{
    //addr_t fpn;
    //int pgit;
    //struct framephy_struct *newfp_str = NULL;

    /* TODO: allocate the page 
    //caller-> ...
    //frm_lst-> ...
    */

    addr_t fpn;
    int pgit;
    struct framephy_struct *head = NULL;
    struct framephy_struct *tail = NULL;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mram == NULL || frm_lst == NULL)
        return -1;

    for (pgit = 0; pgit < req_pgnum; pgit++)
    {
        struct framephy_struct *newfp_str = malloc(sizeof(struct framephy_struct));

        if (newfp_str == NULL)
            return -1;

        // TODO: allocate the page 
        if (MEMPHY_get_freefp(caller->krnl->mram, &fpn) != 0)
        {
            // TODO: ERROR CODE of obtaining somes but not enough frames
            free(newfp_str);
            *frm_lst = head;
            return -3000;
        }

        newfp_str->fpn = fpn;
        newfp_str->fp_next = NULL;
        newfp_str->owner = caller->krnl->mm;

        if (head == NULL)
            head = newfp_str;
        else
            tail->fp_next = newfp_str;

        tail = newfp_str;
    }

    /* End TODO */

    *frm_lst = head;
    return 0;
}

/*
 * vm_map_ram - do the mapping all vm are to ram storage device
 * @caller    : caller
 * @astart    : vm area start
 * @aend      : vm area end
 * @mapstart  : start mapping point
 * @incpgnum  : number of mapped page
 * @ret_rg    : returned region
 */
addr_t vm_map_ram(struct pcb_t *caller, addr_t astart, addr_t aend, addr_t mapstart, int incpgnum, struct vm_rg_struct *ret_rg)
{
    struct framephy_struct *frm_lst = NULL;
    addr_t ret_alloc = 0;
    addr_t mapped_addr = 0;
//int pgnum = incpgnum;

    /*@bksysnet: author provides a feasible solution of getting frames
     *FATAL logic in here, wrong behaviour if we have not enough page
     *i.e. we request 1000 frames meanwhile our RAM has size of 3 frames
     *Don't try to perform that case in this simple work, it will result
     *in endless procedure of swap-off to get frame and we have not provide
     *duplicate control mechanism, keep it simple
     */
    ret_alloc = alloc_pages_range(caller, incpgnum, &frm_lst);

    if (ret_alloc < 0 && ret_alloc != -3000)
        return -1;

    /* Out of memory */
    if (ret_alloc == -3000)
    {
        return -1;
    }

    /* it leaves the case of memory is enough but half in ram, half in swap
     * do the swaping all to swapper to get the all in ram */
     mapped_addr = vmap_page_range(caller, mapstart, incpgnum, frm_lst, ret_rg);
     if (mapped_addr != mapstart)
         return -1;

    return 0;
}

/* Swap copy content page from source frame to destination frame
 * @mpsrc  : source memphy
 * @srcfpn : source physical page number (FPN)
 * @mpdst  : destination memphy
 * @dstfpn : destination physical page number (FPN)
 **/
int __swap_cp_page(struct memphy_struct *mpsrc, addr_t srcfpn,
                                     struct memphy_struct *mpdst, addr_t dstfpn)
{
    int cellidx;
    addr_t addrsrc, addrdst;
    for (cellidx = 0; cellidx < PAGING_PAGESZ; cellidx++)
    {
        addrsrc = srcfpn * PAGING_PAGESZ + cellidx;
        addrdst = dstfpn * PAGING_PAGESZ + cellidx;

        BYTE data;
        MEMPHY_read(mpsrc, addrsrc, &data);
        MEMPHY_write(mpdst, addrdst, data);
    }

    return 0;
}

/*
 *Initialize a empty Memory Management instance
 * @mm:     self mm
 * @caller: mm owner
 */
int init_mm(struct mm_struct *mm, struct pcb_t *caller)
{
    struct vm_area_struct *vma0;

    if (mm == NULL)
        return -1;

    /* TODO init page table directory */
     //mm->pgd = ...
     //mm->p4d = ...
     //mm->pud = ...
     //mm->pmd = ...
     //mm->pt = ...

    mm->pgd = calloc(PAGING64_MAX_PGN, sizeof(addr_t));
    mm->p4d = calloc(PAGING64_MAX_PGN, sizeof(addr_t));
    mm->pud = calloc(PAGING64_MAX_PGN, sizeof(addr_t));
    mm->pmd = calloc(PAGING64_MAX_PGN, sizeof(addr_t));
    mm->pt = calloc(PAGING64_MAX_PGN, sizeof(addr_t));

    if (mm->pgd == NULL || mm->p4d == NULL || mm->pud == NULL || mm->pmd == NULL || mm->pt == NULL)
        return -1;

    memset(mm->symrgtbl, 0, sizeof(mm->symrgtbl));
    mm->fifo_pgn = NULL;
    mm->mem_access_cnt = 0;
    mm->pgtbl_access_cnt = 0;
    mm->page_fault_cnt = 0;
    mm->page_replace_cnt = 0;
    mm->swap_in_cnt = 0;
    mm->swap_out_cnt = 0;
    mm->pgtbl_storage_bytes = (uint64_t)5 * (uint64_t)PAGING64_MAX_PGN * (uint64_t)sizeof(addr_t);
    mm->kcpooltbl = NULL;

    if (caller != NULL && caller->krnl != NULL)
        {
            caller->krnl->pgtbl_storage_bytes += mm->pgtbl_storage_bytes;
        }


    /* By default the owner comes with at least one vma */
    vma0 = malloc(sizeof(struct vm_area_struct));
    if (vma0 == NULL)
        return -1;

    vma0->vm_id = 0;
    vma0->vm_start = 0;
    vma0->vm_end = vma0->vm_start;
    vma0->sbrk = vma0->vm_start;
    struct vm_rg_struct *first_rg = init_vm_rg(vma0->vm_start, vma0->vm_end);
    enlist_vm_rg_node(&vma0->vm_freerg_list, first_rg);

    /* TODO update VMA0 next */
    // vma0->next = ...

    /* Point vma owner backward */
    //vma0->vm_mm = mm; 

    vma0->vm_next = NULL;
    vma0->vm_mm = mm;

    /* TODO: update mmap */
    //mm->mmap = ...
    //mm->symrgtbl = ...
    //mm->kcpooltbl

    mm->mmap = vma0;

    return 0;
}

void print_mm_stats(struct krnl_t *krnl)
{
    if (krnl == NULL || krnl->mm == NULL)
        return;

    printf("MM64 statistics:\n");
    printf("  memory accesses   : %llu\n", (unsigned long long)krnl->mem_access_cnt);
    printf("  pgtbl accesses    : %llu\n", (unsigned long long)krnl->pgtbl_access_cnt);
    printf("  page faults       : %llu\n", (unsigned long long)krnl->page_fault_cnt);
    printf("  page replacements : %llu\n", (unsigned long long)krnl->page_replace_cnt);
    printf("  swap in           : %llu\n", (unsigned long long)krnl->swap_in_cnt);
    printf("  swap out          : %llu\n", (unsigned long long)krnl->swap_out_cnt);
    printf("  pgtbl storage      : %llu bytes\n", (unsigned long long)krnl->pgtbl_storage_bytes);
}

struct vm_rg_struct *init_vm_rg(addr_t rg_start, addr_t rg_end)
{
    struct vm_rg_struct *rgnode = malloc(sizeof(struct vm_rg_struct));

    rgnode->rg_start = rg_start;
    rgnode->rg_end = rg_end;
    rgnode->rg_next = NULL;

    return rgnode;
}

int enlist_vm_rg_node(struct vm_rg_struct **rglist, struct vm_rg_struct *rgnode)
{
    rgnode->rg_next = *rglist;
    *rglist = rgnode;

    return 0;
}

int enlist_pgn_node(struct pgn_t **plist, addr_t pgn)
{
    struct pgn_t *pnode = malloc(sizeof(struct pgn_t));

    pnode->pgn = pgn;
    pnode->pg_next = *plist;
    *plist = pnode;

    return 0;
}

int print_list_fp(struct framephy_struct *ifp)
{
    struct framephy_struct *fp = ifp;

    printf("print_list_fp: ");
    if (fp == NULL) { printf("NULL list\n"); return -1;}
    printf("\n");
    while (fp != NULL)
    {
        printf("fp[" FORMAT_ADDR "]\n", fp->fpn);
        fp = fp->fp_next;
    }
    printf("\n");
    return 0;
}

int print_list_rg(struct vm_rg_struct *irg)
{
    struct vm_rg_struct *rg = irg;

    printf("print_list_rg: ");
    if (rg == NULL) { printf("NULL list\n"); return -1; }
    printf("\n");
    while (rg != NULL)
    {
        printf("rg[" FORMAT_ADDR "->"  FORMAT_ADDR "]\n", rg->rg_start, rg->rg_end);
        rg = rg->rg_next;
    }
    printf("\n");
    return 0;
}

int print_list_vma(struct vm_area_struct *ivma)
{
    struct vm_area_struct *vma = ivma;

    printf("print_list_vma: ");
    if (vma == NULL) { printf("NULL list\n"); return -1; }
    printf("\n");
    while (vma != NULL)
    {
        printf("va[" FORMAT_ADDR "->" FORMAT_ADDR "]\n", vma->vm_start, vma->vm_end);
        vma = vma->vm_next;
    }
    printf("\n");
    return 0;
}

int print_list_pgn(struct pgn_t *ip)
{
    printf("print_list_pgn: ");
    if (ip == NULL) { printf("NULL list\n"); return -1; }
    printf("\n");
    while (ip != NULL)
    {
        printf("va[" FORMAT_ADDR "]-\n", ip->pgn);
        ip = ip->pg_next;
    }
    printf("n");
    return 0;
}

int print_pgtbl(struct pcb_t *caller, addr_t start, addr_t end)
{
//addr_t pgn_start;//, pgn_end;
//addr_t pgit;
//struct krnl_t *krnl = caller->krnl;

    addr_t pgd=0;
    addr_t p4d=0;
    addr_t pud=0;
    addr_t pmd=0;
    addr_t pt=0;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return -1;

    get_pd_from_address(start, &pgd, &p4d, &pud, &pmd, &pt);

    /* TODO traverse the page map and dump the page directory entries */

    if (end == (addr_t)-1)
        end = (addr_t)PAGING64_MAX_PGN * PAGING64_PAGESZ;
    else if (end == 0 || end < start)
        end = start + PAGING64_PAGESZ;

    for (addr_t pgit = (start >> PAGING64_ADDR_PT_SHIFT); pgit < (end >> PAGING64_ADDR_PT_SHIFT); pgit++)
    {
        uint32_t pte = pte_get_entry(caller, pgit);
        if (pte == 0)
            continue;

        get_pd_from_pagenum(pgit, &pgd, &p4d, &pud, &pmd, &pt);
        printf("PGN[" FORMAT_ADDR "] IDX[" FORMAT_ADDR "," FORMAT_ADDR "," FORMAT_ADDR "," FORMAT_ADDR "," FORMAT_ADDR "] PTE[0x%08x] ",
                     pgit, pgd, p4d, pud, pmd, pt, pte);

        if (PAGING_PAGE_PRESENT(pte))
            printf("FPN[" FORMAT_ADDR "]\n", (addr_t)PAGING_FPN(pte));
        else if (pte & PAGING_PTE_SWAPPED_MASK)
            printf("SWP[" FORMAT_ADDR "]\n", (addr_t)PAGING_SWP(pte));
        else
            printf("EMPTY\n");
    }

    return 0;
}

#endif  //def MM64
```

Giải thích: `mm64.c` quản lý bảng trang 5 cấp, cập nhật PTE và thống kê MM64, đồng thời hỗ trợ map frame và in bảng trang.

7. Mã nguồn đầy đủ `mm-vm.c` trong [src/mm-vm.c](src/mm-vm.c):

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Caitoa release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

//#ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
    if (mm == NULL || mm->mmap == NULL)
        return NULL;

    struct vm_area_struct *pvma = mm->mmap;

    while (pvma != NULL && pvma->vm_id < (unsigned long)vmaid)
    {
        pvma = pvma->vm_next;
    }

    return pvma;
}

int __mm_swap_page(struct pcb_t *caller, addr_t vicfpn , addr_t swpfpn)
{
        __swap_cp_page(caller->krnl->mram, vicfpn, caller->krnl->active_mswp, swpfpn);
        return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, addr_t size, addr_t alignedsz)
{
    struct vm_rg_struct * newrg;
    /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
    //struct vm_area_struct *cur_vma = get_vma_by_num(caller->kernl->mm, vmaid);

    //newrg = malloc(sizeof(struct vm_rg_struct));

    /* TODO: update the newrg boundary
    // newrg->rg_start = ...
    // newrg->rg_end = ...
    */
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
    if (cur_vma == NULL)
        return NULL;

    newrg = malloc(sizeof(struct vm_rg_struct));
    if (newrg == NULL)
        return NULL;

    newrg->rg_start = cur_vma->sbrk;
    newrg->rg_end = newrg->rg_start + (alignedsz != 0 ? alignedsz : size);
    /* END TODO */

    return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, addr_t vmastart, addr_t vmaend)
{
    //struct vm_area_struct *vma = caller->krnl->mm->mmap;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return -1;

    /* TODO validate the planned memory area is not overlapped */
    if (vmastart >= vmaend)
    {
        return -1;
    }

    struct vm_area_struct *vma = caller->krnl->mm->mmap;
    if (vma == NULL)
    {
        return -1;
    }

    struct vm_area_struct *cur_area = get_vma_by_num(caller->krnl->mm, vmaid);
    if (cur_area == NULL)
    {
        return -1;
    }

    /* TODO validate the planned memory area is not overlapped */
    while (vma != NULL)
    {
        if (vma != cur_area && !(vmaend <= vma->vm_start || vmastart >= vma->vm_end))
        {
            return -1;
        }
        vma = vma->vm_next;
    }

    /* End TODO*/

    return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, addr_t inc_sz)
{
    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return -1;

    //struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));

    /* TOTO with new address scheme, the size need tobe aligned 
     *      the raw inc_sz maybe not fit pagesize
     */ 
    //addr_t inc_amt;

//  int incnumpage =  inc_amt / PAGING_PAGESZ;

    struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
    if (cur_vma == NULL)
        return -1;

    if (inc_sz == 0)
        return 0;

    addr_t inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);

    /* TODO Validate overlap of obtained region */
    //if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
    //  return -1; /*Overlap and failed allocation */

    addr_t old_end = cur_vma->vm_end;
    addr_t new_end = old_end + inc_amt;

    /* TODO: Obtain the new vm area based on vmaid */
    //cur_vma->vm_end... 
    // inc_limit_ret...
    /* The obtained vm area (only)
     * now will be alloc real ram region */

//  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
//                   old_end, incnumpage , newrg) < 0)
//    return -1; /* Map the memory to MEMRAM */

    if (validate_overlap_vm_area(caller, vmaid, old_end, new_end) < 0)
        return -1;

    cur_vma->vm_end = new_end;
    cur_vma->sbrk = new_end;

    return 0;
}

// #endif
```

Giải thích: `mm-vm.c` quản lý VMA/region, kiểm tra chồng lấn và mở rộng giới hạn vùng nhớ khi cần.

8. Mã nguồn đầy đủ `mm-memphy.c` trong [src/mm-memphy.c](src/mm-memphy.c):

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Caitoa release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Memory physical module mm/mm-memphy.c
 */

#include "mm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_mutex_t memphy_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 *  MEMPHY_mv_csr - move MEMPHY cursor
 *  @mp: memphy struct
 *  @offset: offset
 */
int MEMPHY_mv_csr(struct memphy_struct *mp, addr_t offset)
{
     int numstep = 0;

     mp->cursor = 0;
     while (numstep < offset && numstep < mp->maxsz)
     {
            /* Traverse sequentially */
            mp->cursor = (mp->cursor + 1) % mp->maxsz;
            numstep++;
     }

     return 0;
}

/*
 *  MEMPHY_seq_read - read MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int MEMPHY_seq_read(struct memphy_struct *mp, addr_t addr, BYTE *value)
{
     if (mp == NULL)
            return -1;

     if (!mp->rdmflg)
            return -1; /* Not compatible mode for sequential read */

     MEMPHY_mv_csr(mp, addr);
     *value = (BYTE)mp->storage[addr];

     return 0;
}

/*
 *  MEMPHY_read read MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value
 */
int MEMPHY_read(struct memphy_struct *mp, addr_t addr, BYTE *value)
{
     if (mp == NULL)
            return -1;

     if (addr < 0 || addr >= mp->maxsz)
            return -1;

     if (mp->rdmflg)
            *value = mp->storage[addr];
     else /* Sequential access device */
            return MEMPHY_seq_read(mp, addr, value);

     return 0;
}

/*
 *  MEMPHY_seq_write - write MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int MEMPHY_seq_write(struct memphy_struct *mp, addr_t addr, BYTE value)
{

     if (mp == NULL)
            return -1;

     if (!mp->rdmflg)
            return -1; /* Not compatible mode for sequential read */

     MEMPHY_mv_csr(mp, addr);
     mp->storage[addr] = value;

     return 0;
}

/*
 *  MEMPHY_write-write MEMPHY device
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int MEMPHY_write(struct memphy_struct *mp, addr_t addr, BYTE data)
{
     if (mp == NULL)
            return -1;

     if (addr < 0 || addr >= mp->maxsz)
            return -1;

     if (mp->rdmflg)
            mp->storage[addr] = data;
     else /* Sequential access device */
            return MEMPHY_seq_write(mp, addr, data);

     return 0;
}

/*
 *  MEMPHY_format-format MEMPHY device
 *  @mp: memphy struct
 */
int MEMPHY_format(struct memphy_struct *mp, int pagesz)
{
     /* This setting come with fixed constant PAGESZ */
     int numfp = mp->maxsz / pagesz;
     struct framephy_struct *newfst, *fst;
     int iter = 0;

     if (numfp <= 0)
            return -1;

     /* Init head of free framephy list */
     fst = malloc(sizeof(struct framephy_struct));
     fst->fpn = iter;
     mp->free_fp_list = fst;

     /* We have list with first element, fill in the rest num-1 element member*/
     for (iter = 1; iter < numfp; iter++)
     {
            newfst = malloc(sizeof(struct framephy_struct));
            newfst->fpn = iter;
            newfst->fp_next = NULL;
            fst->fp_next = newfst;
            fst = newfst;
     }

     return 0;
}

int MEMPHY_get_freefp(struct memphy_struct *mp, addr_t *retfpn)
{
     struct framephy_struct *fp;

     if (mp == NULL || retfpn == NULL)
            return -1;

     pthread_mutex_lock(&memphy_lock);

     fp = mp->free_fp_list;

     if (fp == NULL)
     {
            pthread_mutex_unlock(&memphy_lock);
            return -1;
     }

     *retfpn = fp->fpn;
     mp->free_fp_list = fp->fp_next;

     /* MEMPHY is iteratively used up until its exhausted
        * No garbage collector acting then it not been released
        */
     free(fp);

     pthread_mutex_unlock(&memphy_lock);

     return 0;
}

int MEMPHY_dump(struct memphy_struct *mp)
{
    /*TODO dump memphy contnt mp->storage
     *     for tracing the memory content
     */
     return 0;
}

int MEMPHY_put_freefp(struct memphy_struct *mp, addr_t fpn)
{
     struct framephy_struct *fp;
     struct framephy_struct *newnode;

     if (mp == NULL)
            return -1;

     newnode = malloc(sizeof(struct framephy_struct));

     if (newnode == NULL)
            return -1;

     pthread_mutex_lock(&memphy_lock);

     fp = mp->free_fp_list;

     /* Create new node with value fpn */
     newnode->fpn = fpn;
     newnode->fp_next = fp;
     mp->free_fp_list = newnode;

     pthread_mutex_unlock(&memphy_lock);

     return 0;
}

/*
 *  Init MEMPHY struct
 */
int init_memphy(struct memphy_struct *mp, addr_t max_size, int randomflg)
{
     mp->maxsz = max_size;
     mp->storage = NULL;
     if (max_size > 0) {
            mp->storage = (BYTE *)malloc(max_size * sizeof(BYTE));
            if (mp->storage == NULL) return -1;
            memset(mp->storage, 0, max_size * sizeof(BYTE));
     }

     MEMPHY_format(mp, PAGING_PAGESZ);

     mp->rdmflg = (randomflg != 0) ? 1 : 0;

     if (!mp->rdmflg) /* Not Ramdom acess device, then it serial device*/
            mp->cursor = 0;

     return 0;
}

// #endif
```

Giải thích: `mm-memphy.c` trừu tượng hóa MEMRAM/MEMSWAP, cung cấp đọc/ghi và quản lý danh sách frame rảnh có mutex.

9. Mã nguồn đầy đủ `libmem.c` trong [src/libmem.c](src/libmem.c):

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Caitoa release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */

#include "string.h"
#include "mm.h"
#include "mm64.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

static struct pcb_t *
resolve_caller(struct krnl_t *krnl, uint32_t pid)
{
    if (krnl == NULL)
        return NULL;

    return find_proc_by_pid(krnl, pid);
}

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
    struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

    if (rg_elmt->rg_start >= rg_elmt->rg_end)
        return -1;

    if (rg_node != NULL)
        rg_elmt->rg_next = rg_node;

    /* Enlist the new region */
    mm->mmap->vm_freerg_list = rg_elmt;

    return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
    if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
        return NULL;

    return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, addr_t size, addr_t *alloc_addr)
{
    /*Allocate at the toproof */
    pthread_mutex_lock(&mmvm_lock);
    struct vm_rg_struct rgnode;
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
    int inc_sz=0;

    if (cur_vma == NULL)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }

    if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
    {
        caller->krnl->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
        caller->krnl->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
 
        *alloc_addr = rgnode.rg_start;

        pthread_mutex_unlock(&mmvm_lock);
        return 0;
    }

    /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

    /*Attempt to increate limit to get space */
#ifdef MM64
    inc_sz = PAGING64_PAGE_ALIGNSZ(size);
#else
    inc_sz = PAGING_PAGE_ALIGNSZ(size);
#endif
    int old_sbrk;

    old_sbrk = cur_vma->sbrk;

    /* TODO INCREASE THE LIMIT
     * SYSCALL 1 sys_memmap
     */
    struct sc_regs regs;
    regs.a1 = SYSMEM_INC_OP;
    regs.a2 = vmaid;
#ifdef MM64
    regs.a3 = inc_sz;
#else
    regs.a3 = PAGING_PAGE_ALIGNSZ(size);
#endif  
    _syscall(caller->krnl, caller->pid, 17, &regs); /* SYSCALL 17 sys_memmap */

    /*Successful increase limit */
    caller->krnl->mm->symrgtbl[rgid].rg_start = old_sbrk;
    caller->krnl->mm->symrgtbl[rgid].rg_end = old_sbrk + size;

    if (inc_sz > size)
    {
        struct vm_rg_struct *remain_rg = malloc(sizeof(struct vm_rg_struct));
        if (remain_rg != NULL)
        {
            remain_rg->rg_start = old_sbrk + size;
            remain_rg->rg_end = old_sbrk + inc_sz;
            remain_rg->rg_next = NULL;
            enlist_vm_freerg_list(caller->krnl->mm, remain_rg);
        }
    }

    *alloc_addr = old_sbrk;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;

}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
    pthread_mutex_lock(&mmvm_lock);

    if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }

    /* TODO: Manage the collect freed region to freerg_list */
    struct vm_rg_struct *rgnode = get_symrg_byid(caller->krnl->mm, rgid);

    if (rgnode->rg_start == 0 && rgnode->rg_end == 0)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }
    struct vm_rg_struct *freerg_node = malloc(sizeof(struct vm_rg_struct));
    freerg_node->rg_start = rgnode->rg_start;
    freerg_node->rg_end = rgnode->rg_end;
    freerg_node->rg_next = NULL;

    rgnode->rg_start = rgnode->rg_end = 0;
    rgnode->rg_next = NULL;

    /*enlist the obsoleted memory region */
    enlist_vm_freerg_list(caller->krnl->mm, freerg_node);

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct krnl_t *krnl, uint32_t pid, addr_t size, uint32_t reg_index)
{
    addr_t  addr;
    struct pcb_t *proc = resolve_caller(krnl, pid);
    int val;

    if (proc == NULL)
        return -1;

    val = __alloc(proc, 0, reg_index, size, &addr);
    if (val == -1)
    {
        return -1;
    }
#ifdef IODUMP
    /* TODO dump IO content (if needed) */
#ifdef PAGETBL_DUMP
    print_pgtbl(proc, 0, -1); // print max TBL
#endif
#endif

    /* By default using vmaid = 0 */
    return val;
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct krnl_t *krnl, uint32_t pid, uint32_t reg_index)
{
    struct pcb_t *proc = resolve_caller(krnl, pid);
    int val = __free(proc, 0, reg_index);
    if (val == -1)
    {
        return -1;
    }
#ifdef IODUMP
    /* TODO dump IO content (if needed) */
#ifdef PAGETBL_DUMP
    print_pgtbl(proc, 0, -1); // print max TBL
#endif
#endif
    return 0;//val;
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
    uint32_t pte;
    addr_t target_fpn;

    if (mm == NULL || caller == NULL || caller->krnl == NULL || caller->krnl->mram == NULL)
        return -1;

    caller->krnl->mem_access_cnt++;

    pte = pte_get_entry(caller, pgn);

    /* TODO Initialize the target frame storing our variable */
//  addr_t tgtfpn 

    if (!PAGING_PAGE_PRESENT(pte) || (pte & PAGING_PTE_SWAPPED_MASK))
    {
        caller->krnl->page_fault_cnt++;
        addr_t vicpgn = 0;
        addr_t swpfpn = 0;
//  addr_t vicfpn;
//  addr_t vicpte;
//  struct sc_regs regs;

        /* TODO: Play with your paging theory here */
        /* Find victim page */
        if (MEMPHY_get_freefp(caller->krnl->mram, &target_fpn) != 0)
        {
            uint32_t vicpte;
            addr_t vicfpn;

            caller->krnl->page_replace_cnt++;
            if (find_victim_page(caller->krnl->mm, &vicpgn) != 0)
                return -1;

            vicpte = pte_get_entry(caller, vicpgn);
            vicfpn = PAGING_FPN(vicpte);

            /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

            /* TODO copy victim frame to swap 
             * SWP(vicfpn <--> swpfpn)
             * SYSCALL 1 sys_memmap
             */

            if (MEMPHY_get_freefp(caller->krnl->active_mswp, &swpfpn) != 0)
                return -1;

            if (__swap_cp_page(caller->krnl->mram, vicfpn, caller->krnl->active_mswp, swpfpn) != 0)
                return -1;

            caller->krnl->swap_out_cnt++;

            /* Update page table */
            //pte_set_swap(...);

            if (pte_set_swap(caller, vicpgn, caller->krnl->active_mswp_id, swpfpn) != 0)
                return -1;

            /* Update its online status of the target page */
            //pte_set_fpn(...);

            target_fpn = vicfpn;
        }

        if (pte & PAGING_PTE_SWAPPED_MASK)
        {
            addr_t swpoff = PAGING_SWP(pte);

            if (__swap_cp_page(caller->krnl->active_mswp, swpoff, caller->krnl->mram, target_fpn) != 0)
                return -1;

            caller->krnl->swap_in_cnt++;
        }

        if (pte_set_fpn(caller, pgn, target_fpn) != 0)
            return -1;

        enlist_pgn_node(&caller->krnl->mm->fifo_pgn, pgn);
    }

    pte = pte_get_entry(caller, pgn);
    *fpn = PAGING_FPN(pte);

    return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
    int pgn = PAGING_PGN(addr);
//int off = PAGING_OFFST(addr);
    int off = addr & (PAGING64_PAGESZ - 1);
    int fpn;

    if (pg_getpage(mm, pgn, &fpn, caller) != 0)
        return -1; /* invalid page access */

//int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

    /* TODO 
     *  MEMPHY_read(caller->krnl->mram, phyaddr, data);
     *  MEMPHY READ 
     *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
     */
    if (MEMPHY_read(caller->krnl->mram, (addr_t)fpn * PAGING64_PAGESZ + off, data) != 0)
        return -1;

    return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
    int pgn = PAGING_PGN(addr);
//int off = PAGING_OFFST(addr);
    int off = addr & (PAGING64_PAGESZ - 1);
    int fpn;

    /* Get the page to MEMRAM, swap from MEMSWAP if needed */
    if (pg_getpage(mm, pgn, &fpn, caller) != 0)
        return -1; /* invalid page access */

    /* TODO 
     *  MEMPHY_write(caller->krnl->mram, phyaddr, value);
     *  MEMPHY WRITE with SYSMEM_IO_WRITE 
     * SYSCALL 17 sys_memmap
     */
    if (MEMPHY_write(caller->krnl->mram, (addr_t)fpn * PAGING64_PAGESZ + off, value) != 0)
        return -1;

    return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE *data)
{
    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL || data == NULL)
        return -1;

    struct vm_rg_struct *currg = get_symrg_byid(caller->krnl->mm, rgid);
    //struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

    if (currg == NULL || (currg->rg_start == 0 && currg->rg_end == 0) || currg->rg_start + offset >= currg->rg_end)
        return -1;

        /* TODO Invalid memory identify */
    pthread_mutex_lock(&mmvm_lock);
    if (pg_getval(caller->krnl->mm, currg->rg_start + offset, data, caller) != 0)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }
    pthread_mutex_unlock(&mmvm_lock);

    return 0;
}

/*libread - PAGING-based read a region memory */
int libread(
        struct krnl_t *krnl,
        uint32_t pid,
        uint32_t source,    // Index of source register
        addr_t offset,    // Source address = [source] + [offset]
        uint32_t* destination)
{
    struct pcb_t *proc = resolve_caller(krnl, pid);
    BYTE data;
    if (proc == NULL)
        return -1;

    flockfile(stdout);
printf("%s:%d\n",__func__,426);
    funlockfile(stdout);
    int val = __read(proc, 0, source, offset, &data);

    *destination = data;
#ifdef IODUMP
    /* TODO dump IO content (if needed) */
#ifdef PAGETBL_DUMP
    print_pgtbl(proc, 0, -1); // print max TBL
#endif
#endif

    return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE value)
{
    pthread_mutex_lock(&mmvm_lock);
    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }

    struct vm_rg_struct *currg = get_symrg_byid(caller->krnl->mm, rgid);

    struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

    if (currg == NULL || cur_vma == NULL || (currg->rg_start == 0 && currg->rg_end == 0) || currg->rg_start + offset >= currg->rg_end) /* Invalid memory identify */
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }

    if (pg_setval(caller->krnl->mm, currg->rg_start + offset, value, caller) != 0)
    {
        pthread_mutex_unlock(&mmvm_lock);
        return -1;
    }

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
        struct krnl_t *krnl,
        uint32_t pid,
        BYTE data,            // Data to be wrttien into memory
        uint32_t destination, // Index of destination register
        addr_t offset)
{
    struct pcb_t *proc = resolve_caller(krnl, pid);
    if (proc == NULL)
        return -1;

    int val = __write(proc, 0, destination, offset, data);
    if (val == -1)
    {
        return -1;
    }
#ifdef IODUMP
    /* TODO dump IO content (if needed) */
#ifdef PAGETBL_DUMP
    print_pgtbl(proc, 0, -1); // print max TBL
#endif
#endif

    return val;
}


/*libkmem_malloc- alloc region memory in kmem
 *@caller: caller
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: memory size
 */

int libkmem_malloc(struct krnl_t *krnl, uint32_t pid, uint32_t size, uint32_t reg_index)
{
    struct pcb_t *caller = resolve_caller(krnl, pid);
    addr_t addr = 0;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return -1;

    if (__kmalloc(caller, 0, reg_index, size, &addr) == (addr_t)-1)
        return -1;

    if (caller->krnl->mm->symrgtbl[reg_index].rg_end <= caller->krnl->mm->symrgtbl[reg_index].rg_start)
        return -1;

    return 0;
}


/*kmalloc - alloc region memory in kmem
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: memory size
 *@alloc_addr: allocated address
 */
addr_t __kmalloc(struct pcb_t *caller, int vmaid, int rgid, addr_t size, addr_t *alloc_addr)
{
    int ret;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL || alloc_addr == NULL)
        return (addr_t)-1;

    ret = __alloc(caller, vmaid, rgid, size, alloc_addr);
    if (ret != 0)
        return (addr_t)-1;

    return *alloc_addr;

}

/*libkmem_cache_pool_create - create cache pool in kmem
 *@caller: caller
 *@size: memory size
 *@align: alignment size of each cache slot (identical cache slot size)
 *@cache_pool_id: cache pool ID
 */
int libkmem_cache_pool_create(struct krnl_t *krnl, uint32_t pid, uint32_t size, uint32_t align, uint32_t cache_pool_id)
{
    /* TODO: provide OS level management */

    //struct krnl_t *krnl = caller->krnl;
    //krnl->kcpooltbl...
    //krnl->krnl_pgd ...
  
    struct kcache_pool_struct *newtbl;
    addr_t storage = 0;

    struct pcb_t *caller = resolve_caller(krnl, pid);

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL || align == 0)
        return -1;

    newtbl = realloc(caller->krnl->mm->kcpooltbl, (cache_pool_id + 1) * sizeof(struct kcache_pool_struct));
    if (newtbl == NULL)
        return -1;

    caller->krnl->mm->kcpooltbl = newtbl;
    memset(&caller->krnl->mm->kcpooltbl[cache_pool_id], 0, sizeof(struct kcache_pool_struct));
    caller->krnl->mm->kcpooltbl[cache_pool_id].align = (int)align;
    caller->krnl->mm->kcpooltbl[cache_pool_id].size = (int)size;

    if (__kmalloc(caller, 0, (int)(PAGING_MAX_SYMTBL_SZ - 1 - cache_pool_id), size, &storage) == (addr_t)-1)
        return -1;

    caller->krnl->mm->kcpooltbl[cache_pool_id].storage = storage;

    return 0;
}

/*libkmem_cache_alloc - allocate cache slot in cache pool, cache slot has identical size
 * the allocated size is embedded in pool management mechanism
 *@caller: caller
 *@cache_pool_id: cache pool ID
 *@reg_index: memory region index
 */
int libkmem_cache_alloc(struct krnl_t *krnl, uint32_t pid, uint32_t cache_pool_id, uint32_t reg_index)
{
    /* TODO: provide OS level management
     *       and forward the request to helper
     */
    struct pcb_t *proc = resolve_caller(krnl, pid);
    addr_t addr = 0;

    if (proc == NULL)
        return -1;

    if (__kmem_cache_alloc(proc, 0, reg_index, (int)cache_pool_id, &addr) == (addr_t)-1)
        return -1;

    return 0;
}

/*kmem_cache_alloc - alloc region memory in kmem cache
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@cache_pool_id: cached pool ID
 *@alloc_addr: allocated address
 */

addr_t __kmem_cache_alloc(struct pcb_t *caller, int vmaid, int rgid, int cache_pool_id, addr_t *alloc_addr)
{
    /* TODO: provide OS level management */
    /* TODO: provide OS level management */

    //struct krnl_t *krnl = caller->krnl;
    //krnl->symrgtbl...
    //krnl->kcpooltbl...
    //krnl->krnl_pgd ...
    struct kcache_pool_struct *pool;

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL || alloc_addr == NULL)
        return (addr_t)-1;

    if (caller->krnl->mm->kcpooltbl == NULL || cache_pool_id < 0)
        return (addr_t)-1;

    pool = &caller->krnl->mm->kcpooltbl[cache_pool_id];
    if (pool->align <= 0 || pool->size < pool->align)
        return (addr_t)-1;

    *alloc_addr = pool->storage;
    pool->storage += (addr_t)pool->align;
    pool->size -= pool->align;

    caller->krnl->mm->symrgtbl[rgid].rg_start = *alloc_addr;
    caller->krnl->mm->symrgtbl[rgid].rg_end = *alloc_addr + (addr_t)pool->align;

    return *alloc_addr;

}


int libkmem_copy_from_user(struct krnl_t *krnl, uint32_t pid, uint32_t source, uint32_t destination, uint32_t offset, uint32_t size)
{
     /* TODO: provide OS level management kmem
     */
    /*
     * TODO: Map kernel address range
     */
    //__read_user_mem(...)
    //__write_kernel_mem(...);
    uint32_t i;
    BYTE data_cell;

    struct pcb_t *caller = resolve_caller(krnl, pid);

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return -1;

    for (i = 0; i < size; i++)
    {
        if (__read_user_mem(caller, 0, source, offset + i, &data_cell) != 0)
            return -1;

        if (__write_kernel_mem(caller, 0, destination, offset + i, data_cell) != 0)
            return -1;
    }

    return 0;
}

int libkmem_copy_to_user(struct krnl_t *krnl, uint32_t pid, uint32_t source, uint32_t destination, uint32_t offset, uint32_t size)
{
    /* TODO: provide OS level management kmem
     */
    /*
     * TODO: Map kernel address range
     */
    //__read_kernel_mem(...)
    //__write_user_mem(...);
    uint32_t i;
    BYTE data_cell;

    struct pcb_t *caller = resolve_caller(krnl, pid);

    if (caller == NULL || caller->krnl == NULL || caller->krnl->mm == NULL)
        return -1;

    for (i = 0; i < size; i++)
    {
        if (__read_kernel_mem(caller, 0, source, offset + i, &data_cell) != 0)
            return -1;

        if (__write_user_mem(caller, 0, destination, offset + i, data_cell) != 0)
            return -1;
    }

    return 0;
}


/*__read_kernel_mem - read value in kernel region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@offset: offset to acess in memory region
 *@value: data value
 */
int __read_kernel_mem(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE *data)
{
    /* TODO: provide OS memory operator for kernel memory region */
    //krnl->krnl_pgd ... or krnl->pgd ... based on kmem implementation strategy
    return __read(caller, vmaid, rgid, offset, data);
}

/*__write_kernel_mem - write a kernel region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@offset: offset to acess in memory region
 *@value: data value
 */
int __write_kernel_mem(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE value)
{
    /* TODO: provide OS memory operator for kernel memory region */
    //krnl->krnl_pgd ... or krnl->pgd ... based on kmem implementation strategy
    return __write(caller, vmaid, rgid, offset, value);
}

/*__read_user_mem - read value in user region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@offset: offset to acess in memory region
 *@value: data value
 */
int __read_user_mem(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE *data)
{
    /* TODO: provide OS level management user memory access */
    //krnl->pgd ...
    return __read(caller, vmaid, rgid, offset, data);
}


/*__write_user_mem - write a user region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@offset: offset to acess in memory region
 *@value: data value
 */
int __write_user_mem(struct pcb_t *caller, int vmaid, int rgid, addr_t offset, BYTE value)
{
    /* TODO: provide OS level management user memory access */
    //krnl->pgd ...
    return __write(caller, vmaid, rgid, offset, value);
}


/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
    pthread_mutex_lock(&mmvm_lock);
    int pagenum, fpn;
    uint32_t pte;

    for (pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
    {
        pte = pte_get_entry(caller, pagenum);

        if (PAGING_PAGE_PRESENT(pte))
        {
            fpn = PAGING_FPN(pte);
            MEMPHY_put_freefp(caller->krnl->mram, fpn);
        }
        else
        {
            fpn = PAGING_SWP(pte);
            MEMPHY_put_freefp(caller->krnl->active_mswp, fpn);
        }
    }

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
}


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, addr_t *retpgn)
{
    struct pgn_t *pg;

    /* TODO: Implement the theorical mechanism to find the victim page */
    if (mm == NULL || retpgn == NULL)
    {
        return -1;
    }

    pg = mm->fifo_pgn;
    if (pg == NULL)
        return -1;


    if (pg->pg_next == NULL)
    {
        *retpgn = pg->pgn;
        mm->fifo_pgn = NULL;
        free(pg);
        return 0;
    }

    struct pgn_t *prev = NULL;
    while (pg->pg_next != NULL)
    {
        prev = pg;
        pg = pg->pg_next;
    }

    *retpgn = pg->pgn;
    if (prev != NULL)
        prev->pg_next = NULL;

    free(pg);

    return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

    struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

    if (rgit == NULL)
        return -1;

    /* Probe unintialized newrg */
    newrg->rg_start = newrg->rg_end = -1;

    /* Traverse on list of free vm region to find a fit space */
    while (rgit != NULL)
    {
        if (rgit->rg_start + size <= rgit->rg_end)
        { /* Current region has enough space */
            newrg->rg_start = rgit->rg_start;
            newrg->rg_end = rgit->rg_start + size;

            /* Update left space in chosen region */
            if (rgit->rg_start + size < rgit->rg_end)
            {
                rgit->rg_start = rgit->rg_start + size;
            }
            else
            { /*Use up all space, remove current node */
                /*Clone next rg node */
                struct vm_rg_struct *nextrg = rgit->rg_next;

                /*Cloning */
                if (nextrg != NULL)
                {
                    rgit->rg_start = nextrg->rg_start;
                    rgit->rg_end = nextrg->rg_end;

                    rgit->rg_next = nextrg->rg_next;

                    free(nextrg);
                }
                else
                {                                /*End of free list */
                    rgit->rg_start = rgit->rg_end; // dummy, size 0 region
                    rgit->rg_next = NULL;
                }
            }
            break;
        }
        else
        {
            rgit = rgit->rg_next; // Traverse next rg
        }
    }

    if (newrg->rg_start == -1) // new region not found
        return -1;

    return 0;
}

// #endif
```

Giải thích: `libmem.c` hiện thực API cấp phát/giải phóng/đọc/ghi, xử lý page fault và hỗ trợ kmem.

### 2.2.3 Kết quả kiểm thử

So sánh thống kê MM64 giữa cấu hình 1K và 4K:

| Cấu hình | mem_access | pgtbl_access | page_fault | pgtbl_storage |
| --- | --- | --- | --- | --- |
| small_1K | 3 | 7 | 1 | 163840 bytes |
| small_4K | 4 | 9 | 1 | 163840 bytes |

**Bảng 2:** Thống kê MM64 giữa cấu hình 1K và 4K.

### 2.2.4 Trả lời câu hỏi

**Câu hỏi:** Lợi ích của thiết kế nhiều vùng nhớ (segments) trong hệ thống này là gì?

**Trả lời:** Thiết kế nhiều vùng nhớ giúp tách logic code/data/heap, hỗ trợ kiểm soát quyền truy cập và giảm rủi ro truy cập sai vùng. Cấu trúc `vm_area_struct` và danh sách region trống trong [src/mm-vm.c](src/mm-vm.c) cho phép mở rộng linh hoạt mà không cần cấp phát liên tục.

**Câu hỏi:** Điều gì xảy ra khi mở rộng phân trang lên nhiều cấp hơn 2?

**Trả lời:** Nhiều cấp giúp bảng trang thưa và tiết kiệm bộ nhớ metadata do chỉ cấp phát cấp con khi cần. Nhược điểm là tăng độ trễ dịch địa chỉ vì phải đi qua nhiều tầng (MM64 là 5 tầng), được phản ánh bởi `pgtbl_access` trong thống kê MM64.

**Câu hỏi:** Ưu/nhược điểm của segmentation kết hợp paging?

**Trả lời:** Ưu điểm là quản lý logic vùng nhớ rõ ràng và giảm phân mảnh ngoại vi nhờ paging; nhược điểm là tăng chi phí tra bảng và yêu cầu phần cứng hỗ trợ nhiều cấp dịch địa chỉ.

---

## 2.3 System Call

### 2.3.1 Cấu trúc

System call là giao diện giữa user space và kernel. Bảng syscall được sinh từ `syscalltbl.lst` và dispatch bởi `_syscall()` trong [src/syscall.c](src/syscall.c). Hiện tại chỉ có 2 syscall active: `sys_listsyscall` và `sys_memmap`.

**Hình 7:** Sơ đồ giao tiếp System Call và kernel.

### 2.3.2 Hiện thực

- Liệt kê syscall bằng `__sys_listsyscall()` trong [src/sys_listsyscall.c](src/sys_listsyscall.c).
- Quản lý bộ nhớ qua `__sys_memmap()` trong [src/sys_mem.c](src/sys_mem.c), hỗ trợ `SYSMEM_MAP_OP`, `SYSMEM_INC_OP`, `SYSMEM_SWP_OP`, `SYSMEM_IO_READ`, `SYSMEM_IO_WRITE`.

1. Hàm dispatch syscall trong [src/syscall.c](src/syscall.c):

```c
#define __SYSCALL(nr, sym) case nr: return __##sym(krnl,pid,regs);
int _syscall(struct krnl_t *krnl, uint32_t pid, uint32_t nr, struct sc_regs* regs)
{
	switch (nr) {
	#include "syscalltbl.lst"
	default: return __sys_ni_syscall(krnl, regs);
	}
};
```

Giải thích: Mã syscall `nr` được ánh xạ qua bảng `syscalltbl.lst` và chuyển tới handler tương ứng.

2. Hàm `__sys_memmap()` trong [src/sys_mem.c](src/sys_mem.c):

```c
int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    int memop;
    BYTE value;
    struct pcb_t *caller;
    int rc = -1;

     if (krnl == NULL || regs == NULL)
         {
             return -1;
         }

     pthread_mutex_lock (&sysmem_lock);

     memop = regs->a1;
     caller = find_proc_by_pid (krnl, pid);

     if (caller == NULL)
         {
             goto out;
         }

     if (caller->krnl == NULL || caller->krnl->mram == NULL)
         {
             goto out;
         }

   switch (memop) {
   case SYSMEM_MAP_OP:
            /* Reserved process case*/
			rc = vmap_pgd_memset (caller, regs->a2, regs->a3);
			break;
   case SYSMEM_INC_OP:
		    rc = inc_vma_limit (caller, regs->a2, regs->a3);
		    break;
   case SYSMEM_SWP_OP:
		    rc = __mm_swap_page (caller, regs->a2, regs->a3);
		    break;
   case SYSMEM_IO_READ:
		    MEMPHY_read (caller->krnl->mram, regs->a2, &value);
            regs->a3 = value;
			    rc = 0;
		    break;
   case SYSMEM_IO_WRITE:
		    MEMPHY_write (caller->krnl->mram, regs->a2, regs->a3);
			    rc = 0;
		    break;
   default:
            printf("Memop code: %d\n", memop);
                                                rc = -1;
   }

out:
         pthread_mutex_unlock (&sysmem_lock);
         return rc;
}
```

Giải thích: `sys_memmap` là syscall lõi cho quản lý bộ nhớ, hỗ trợ map, mở rộng VMA, swap, và I/O đọc/ghi vào MEMPHY.

3. Hàm `__sys_listsyscall()` trong [src/sys_listsyscall.c](src/sys_listsyscall.c):

```c
int
__sys_listsyscall (struct krnl_t *krnl, uint32_t pid, struct sc_regs *regs)
{
    FILE *fp;
    char line[256];
    int nr;
    char sym[128];

    if (krnl == NULL || regs == NULL)
        {
            return -1;
        }

    fp = fopen ("src/syscalltbl.lst", "r");
    if (fp == NULL)
        {
            printf ("Kernel Error: Cannot open system call vector list table.\n");
            return -1;
        }

    printf ("==================================================\n");
    printf ("KERNEL INTERFACE: ACTIVE SYSTEM CALL VECTOR TABLE\n");
    printf ("==================================================\n");

    while (fgets (line, sizeof (line), fp) != NULL)
        {
            if (sscanf (line, "__SYSCALL(%d, %127[^)])", &nr, sym) == 2)
                {
                    printf ("  %d %s\n", nr, sym);
                }
        }

    printf ("==================================================\n");

    fclose (fp);
    return 0;
}
```

Giải thích: Kernel đọc bảng `syscalltbl.lst` và in danh sách syscall đang kích hoạt.

4. Mã nguồn đầy đủ `syscall.c` trong [src/syscall.c](src/syscall.c):

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Caitoa release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "syscall.h"
#include "common.h"

#define __SYSCALL(nr, sym) extern int __##sym(struct krnl_t*, uint32_t,struct sc_regs*);
#include "syscalltbl.lst"
#undef  __SYSCALL

/*
 * The sys_call_table[] is used for system calls, but to know the system
 * call address.
 */
#define __SYSCALL(nr, sym) #nr "-" #sym,
const char* sys_call_table[] = {
#include "syscalltbl.lst"
};
#undef  __SYSCALL
const int syscall_table_size = sizeof(sys_call_table)/sizeof(char*);

int __sys_ni_syscall(struct krnl_t *krnl, struct sc_regs *regs)
{
   /*
    * DUMMY systemcall
    */

   return 0;
}

#define __SYSCALL(nr, sym) case nr: return __##sym(krnl,pid,regs);
int _syscall(struct krnl_t *krnl, uint32_t pid, uint32_t nr, struct sc_regs* regs)
{
	switch (nr) {
	#include "syscalltbl.lst"
	default: return __sys_ni_syscall(krnl, regs);
	}
};
```

Giải thích: `syscall.c` tạo bảng syscall và thực hiện dispatch theo mã `nr` qua `syscalltbl.lst`.

5. Mã nguồn đầy đủ `sys_mem.c` trong [src/sys_mem.c](src/sys_mem.c):

```c
/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Caitoa release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "os-mm.h"
#include "syscall.h"
#include "libmem.h"
#include "queue.h"
#include "sched.h"
#include <stdlib.h>
#include <pthread.h>

#ifdef MM64
#include "mm64.h"
#else
#include "mm.h"
#endif

static pthread_mutex_t sysmem_lock = PTHREAD_MUTEX_INITIALIZER;

int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
    int memop;
    BYTE value;
    struct pcb_t *caller;
    int rc = -1;

    /* TODO THIS DUMMY CREATE EMPTY PROC TO AVOID COMPILER NOTIFY 
     *      need to be eliminated
	*/

     /*
        * @bksysnet: Please note in the dual spacing design
        *            syscall implementations are in kernel space.
        */

          /* TODO: Traverse proclist to terminate the proc
    *       stcmp to check the process match proc_name
    */
//	struct queue_t *running_list = krnl->running_list;
     if (krnl == NULL || regs == NULL)
         {
             return -1;
         }

     pthread_mutex_lock (&sysmem_lock);

     memop = regs->a1;
     caller = find_proc_by_pid (krnl, pid);

     if (caller == NULL)
         {
             goto out;
         }

     if (caller->krnl == NULL || caller->krnl->mram == NULL)
         {
             goto out;
         }

     /* TODO Maching and marking the process */
     /* user process are not allowed to access directly pcb in kernel space of syscall */
     //....

   switch (memop) {
   case SYSMEM_MAP_OP:
            /* Reserved process case*/
			rc = vmap_pgd_memset (caller, regs->a2, regs->a3);
			break;
   case SYSMEM_INC_OP:
		    rc = inc_vma_limit (caller, regs->a2, regs->a3);
		    break;
   case SYSMEM_SWP_OP:
		    rc = __mm_swap_page (caller, regs->a2, regs->a3);
		    break;
   case SYSMEM_IO_READ:
		    MEMPHY_read (caller->krnl->mram, regs->a2, &value);
            regs->a3 = value;
			    rc = 0;
		    break;
   case SYSMEM_IO_WRITE:
		    MEMPHY_write (caller->krnl->mram, regs->a2, regs->a3);
			    rc = 0;
		    break;
   default:
            printf("Memop code: %d\n", memop);
                                                rc = -1;
   }

out:
         pthread_mutex_unlock (&sysmem_lock);
         return rc;
}
```

Giải thích: `sys_mem.c` xử lý syscall quản lý bộ nhớ, gọi các thao tác map, mở rộng VMA, swap và I/O vật lý.

6. Mã nguồn đầy đủ `sys_listsyscall.c` trong [src/sys_listsyscall.c](src/sys_listsyscall.c):

```c
/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "syscall.h"
#include <stdio.h>
#include <stdlib.h>

int
__sys_listsyscall (struct krnl_t *krnl, uint32_t pid, struct sc_regs *regs)
{
    FILE *fp;
    char line[256];
    int nr;
    char sym[128];

    if (krnl == NULL || regs == NULL)
        {
            return -1;
        }

    fp = fopen ("src/syscalltbl.lst", "r");
    if (fp == NULL)
        {
            printf ("Kernel Error: Cannot open system call vector list table.\n");
            return -1;
        }

    printf ("==================================================\n");
    printf ("KERNEL INTERFACE: ACTIVE SYSTEM CALL VECTOR TABLE\n");
    printf ("==================================================\n");

    while (fgets (line, sizeof (line), fp) != NULL)
        {
            if (sscanf (line, "__SYSCALL(%d, %127[^)])", &nr, sym) == 2)
                {
                    printf ("  %d %s\n", nr, sym);
                }
        }

    printf ("==================================================\n");

    fclose (fp);
    return 0;
}
```

Giải thích: `sys_listsyscall.c` đọc `syscalltbl.lst` để in danh sách syscall đang active.

### 2.3.3 Kết quả kiểm thử

Kết quả từ [output/os_syscall_list.output](output/os_syscall_list.output):

```text
KERNEL INTERFACE: ACTIVE SYSTEM CALL VECTOR TABLE
  0 sys_listsyscall
  17 sys_memmap
```

### 2.3.4 Trả lời câu hỏi

**Câu hỏi:** Cơ chế truyền đối số phức tạp cho syscall là gì?

**Trả lời:** Dùng pass-by-address, truyền con trỏ đến vùng nhớ chứa dữ liệu. Kernel đọc vùng nhớ đó và xử lý, tương tự cách `libmem` thao tác thông qua syscall 17 trong [src/libmem.c](src/libmem.c).

**Câu hỏi:** Nếu syscall chạy quá lâu thì sao?

**Trả lời:** CPU có thể bị chiếm dụng lâu, giảm tính đáp ứng và tăng nguy cơ starvation. Do đó cần giới hạn thời gian hoặc tách xử lý dài thành các bước có thể preempt được.

---

## 2.4 Put it all together

Pipeline tổng thể: Loader nạp tiến trình → Scheduler chọn tiến trình → CPU thực thi → Memory/ Syscall xử lý truy cập. Toàn bộ kịch bản được chạy bằng [run.sh](run.sh), kết quả được đối chứng trong [output/](output/).

**Hình 8:** Sơ đồ tích hợp các module Scheduler - Memory - Syscall.

---

# Tài liệu tham khảo

[1] A. Silberschatz, P. B. Galvin and G. Gagne, Operating System Concepts, 10 edition.  
[2] Faculty of Computer Science and Engineering, HCMUT, Operating Systems - Lab Manual (CO2018).