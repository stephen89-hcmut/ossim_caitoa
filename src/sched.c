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
static struct queue_t running_list;
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;

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

	return empty (&run_queue);
#else
	return (empty (&ready_queue) && empty (&run_queue));
#endif
}

void
init_scheduler (void)
{
	ready_queue.size = 0;
	run_queue.size = 0;
	running_list.size = 0;

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

	if (proc == NULL)
		{
			proc = dequeue (&run_queue);
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
	enqueue (&run_queue, proc);
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

void
finish_scheduler (void)
{
}
#endif


