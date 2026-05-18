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

#ifdef MM64
#include "mm64.h"
#else
#include "mm.h"
#endif

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
find_proc_by_pid (struct krnl_t *krnl, uint32_t pid)
{
     struct pcb_t *caller;
     int i;

     /* TODO: Traverse proclist to locate the process by pid
      *       and keep the syscall side free from user-space PCB access.
      */

     if (krnl == NULL)
         {
             return NULL;
         }

     caller = find_proc_by_pid_in_queue (krnl->ready_queue, pid);
     if (caller != NULL)
         {
             return caller;
         }

     caller = find_proc_by_pid_in_queue (krnl->running_list, pid);
     if (caller != NULL)
         {
             return caller;
         }

#ifdef MLQ_SCHED
     if (krnl->mlq_ready_queue != NULL)
         {
             for (i = 0; i < MAX_PRIO; i++)
                 {
                     caller = find_proc_by_pid_in_queue (&krnl->mlq_ready_queue[i], pid);
                     if (caller != NULL)
                         {
                             return caller;
                         }
                 }
         }
#endif

     return NULL;
}

int __sys_memmap(struct krnl_t *krnl, uint32_t pid, struct sc_regs* regs)
{
   int memop = regs->a1;
   BYTE value;

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
     struct pcb_t *caller = find_proc_by_pid (krnl, pid);

     if (caller == NULL)
         {
             return -1;
         }

     /* TODO Maching and marking the process */
     /* user process are not allowed to access directly pcb in kernel space of syscall */
     //....

   switch (memop) {
   case SYSMEM_MAP_OP:
            /* Reserved process case*/
			return vmap_pgd_memset (caller, regs->a2, regs->a3);
   case SYSMEM_INC_OP:
		    return inc_vma_limit (caller, regs->a2, regs->a3);
   case SYSMEM_SWP_OP:
		    return __mm_swap_page (caller, regs->a2, regs->a3);
   case SYSMEM_IO_READ:
		    MEMPHY_read (caller->krnl->mram, regs->a2, &value);
            regs->a3 = value;
                        return 0;
   case SYSMEM_IO_WRITE:
		    MEMPHY_write (caller->krnl->mram, regs->a2, regs->a3);
                        return 0;
   default:
            printf("Memop code: %d\n", memop);
                        return -1;
   }
}


