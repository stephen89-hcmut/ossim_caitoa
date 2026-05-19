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


