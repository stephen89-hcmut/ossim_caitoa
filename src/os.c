
#include "cpu.h"
#include "timer.h"
#include "sched.h"
#include "loader.h"
#include "mm.h"
#ifdef MM64
#include "mm64.h"
#endif

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int time_slot;
static int num_cpus;
static int done = 0;
static struct krnl_t os;

#ifdef MM_PAGING
static unsigned long memramsz;
static unsigned long memswpsz[PAGING_MAX_MMSWP];

struct mmpaging_ld_args {
	/* A dispatched argument struct to compact many-fields passing to loader */
	int vmemsz;
	struct memphy_struct *mram;
	struct memphy_struct **mswp;
	struct memphy_struct *active_mswp;
	int active_mswp_id;
	struct timer_id_t  *timer_id;
};
#endif

static struct ld_args{
	char ** path;
	unsigned long * start_time;
#ifdef MLQ_SCHED
	unsigned long * prio;
#endif
} ld_processes;
int num_processes;

struct cpu_args {
	struct timer_id_t * timer_id;
	int id;
};

static int read_nonempty_line(FILE * file, char * buffer, size_t size)
{
	while (fgets(buffer, size, file) != NULL) {
		char * cursor = buffer;

		while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r')
			cursor++;

		if (*cursor == '\0' || *cursor == '\n')
			continue;

		return 1;
	}

	return 0;
}

static int parse_process_line(const char * line,
			      unsigned long * start_time,
			      char * proc,
			      unsigned long * prio)
{
	char start_buf[100];
	char proc_buf[100];
	char prio_buf[100];
	char * endptr;
	int scanned;

	if (prio != NULL) {
		scanned = sscanf(line, "%99s %99s %99s", start_buf, proc_buf, prio_buf);
		if (scanned < 3)
			return 0;
	} else {
		scanned = sscanf(line, "%99s %99s", start_buf, proc_buf);
		if (scanned < 2)
			return 0;
	}

	*start_time = strtoul(start_buf, &endptr, 10);
	if (*endptr != '\0')
		return 0;

	snprintf(proc, 100, "%s", proc_buf);

	if (prio != NULL) {
		*prio = strtoul(prio_buf, &endptr, 10);
		if (*endptr != '\0')
			return 0;
	}

	return 1;
}


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
			printf("\tCPU %d: Processed %2d has finished\n",
				id ,proc->pid);
			free(proc);
			proc = get_proc();
			time_left = 0;
		}else if (time_left == 0) {
			/* The process has done its job in current time slot */
			printf("\tCPU %d: Put process %2d to run queue\n",
				id, proc->pid);
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
			printf("\tCPU %d: Dispatched process %2d\n",
				id, proc->pid);
			time_left = time_slot;
		}
		
		/* Run current process */
		run(proc);
		time_left--;
		next_slot(timer_id);
	}
	detach_event(timer_id);
	pthread_exit(NULL);
}

static void * ld_routine(void * args) {
#ifdef MM_PAGING
	struct memphy_struct* mram = ((struct mmpaging_ld_args *)args)->mram;
	struct memphy_struct** mswp = ((struct mmpaging_ld_args *)args)->mswp;
	struct memphy_struct* active_mswp = ((struct mmpaging_ld_args *)args)->active_mswp;
	struct timer_id_t * timer_id = ((struct mmpaging_ld_args *)args)->timer_id;
#else
	struct timer_id_t * timer_id = (struct timer_id_t*)args;
#endif
	int i = 0;
  /* TODO init kernel page table directory */
#ifdef MM64
	os.krnl_pgd = malloc(PAGING64_MAX_PGN * sizeof(addr_t));
	os.krnl_p4d = malloc(PAGING64_MAX_PGN * sizeof(addr_t));
	os.krnl_pud = malloc(PAGING64_MAX_PGN * sizeof(addr_t));
	os.krnl_pmd = malloc(PAGING64_MAX_PGN * sizeof(addr_t));
	os.krnl_pt = malloc(PAGING64_MAX_PGN * sizeof(addr_t));

	for (i = 0; i < PAGING64_MAX_PGN; i++)
	{
	   os.krnl_pgd[i] = (addr_t)&os.krnl_p4d;
	   os.krnl_p4d[i] = (addr_t)&os.krnl_pud;
	   os.krnl_pud[i] = (addr_t)&os.krnl_pmd;
	   os.krnl_pmd[i] = (addr_t)&os.krnl_pt;
	   os.krnl_pt[i] = 0;
	}
#else
	os.krnl_pgd = malloc(PAGING_MAX_PGN * sizeof(uint32_t));
#endif
	i=0;
	printf("ld_routine\n");
	while (i < num_processes) {
		struct pcb_t * proc = load(ld_processes.path[i]);
		struct krnl_t * krnl = proc->krnl = &os;	

#ifdef MLQ_SCHED
		proc->prio = ld_processes.prio[i];
#endif
		while (current_time() < ld_processes.start_time[i]) {
			next_slot(timer_id);
		}
#ifdef MM_PAGING
		krnl->mm = malloc(sizeof(struct mm_struct));
		init_mm(krnl->mm, proc);
		krnl->mram = mram;
		krnl->mswp = mswp;
		krnl->active_mswp = active_mswp;
#endif
		printf("\tLoaded a process at %s, PID: %d PRIO: %ld\n",
			ld_processes.path[i], proc->pid, ld_processes.prio[i]);
		add_proc(proc);
		free(ld_processes.path[i]);
		i++;
		next_slot(timer_id);
	}
	free(ld_processes.path);
	free(ld_processes.start_time);
	done = 1;
	detach_event(timer_id);
	pthread_exit(NULL);
}

static void read_config(const char * path) {
	FILE * file;
	if ((file = fopen(path, "r")) == NULL) {
		printf("Cannot find configure file at %s\n", path);
		exit(1);
	}
	fscanf(file, "%d %d %d\n", &time_slot, &num_cpus, &num_processes);
	ld_processes.path = (char**)malloc(sizeof(char*) * num_processes);
	ld_processes.start_time = (unsigned long*)
		malloc(sizeof(unsigned long) * num_processes);
#ifdef MM_PAGING
	int sit;
	char line[256];
	char pending_line[256];
	int has_pending_line = 0;
	int has_mem_line = 0;
	char mem_tok_0[100];
	char mem_tok_1[100];
	char mem_tok_2[100];
	char mem_tok_3[100];
	char mem_tok_4[100];
	char * endptr;

	memramsz = 0x100000000;
	memswpsz[0] = 0x1000000;
	for (sit = 1; sit < PAGING_MAX_MMSWP; sit++)
		memswpsz[sit] = 0;

	if (read_nonempty_line(file, line, sizeof(line))) {
		if (sscanf(line, "%99s %99s", mem_tok_0, mem_tok_1) == 2) {
			unsigned long first = strtoul(mem_tok_0, &endptr, 10);
			if (*endptr == '\0') {
				unsigned long second = strtoul(mem_tok_1, &endptr, 10);
				if (*endptr == '\0') {
					mem_tok_2[0] = '\0';
					mem_tok_3[0] = '\0';
					mem_tok_4[0] = '\0';
					(void)sscanf(line, "%99s %99s %99s %99s %99s",
						mem_tok_0, mem_tok_1, mem_tok_2, mem_tok_3, mem_tok_4);
					memramsz = first;
					memswpsz[0] = second;
					for (sit = 1; sit < PAGING_MAX_MMSWP; sit++)
						memswpsz[sit] = 0;
					if (mem_tok_2[0] != '\0')
						memswpsz[1] = strtoul(mem_tok_2, NULL, 10);
					if (mem_tok_3[0] != '\0')
						memswpsz[2] = strtoul(mem_tok_3, NULL, 10);
					if (mem_tok_4[0] != '\0')
						memswpsz[3] = strtoul(mem_tok_4, NULL, 10);
					has_mem_line = 1;
				} else {
					has_pending_line = 1;
					snprintf(pending_line, sizeof(pending_line), "%s", line);
				}
			} else {
				has_pending_line = 1;
				snprintf(pending_line, sizeof(pending_line), "%s", line);
			}
		}
	}

	if (!has_mem_line && !has_pending_line) {
		/* No extra line was consumed yet; the next line will be a process line. */
	}
#endif

#ifdef MLQ_SCHED
	ld_processes.prio = (unsigned long*)
		malloc(sizeof(unsigned long) * num_processes);
#endif
	int i;
	for (i = 0; i < num_processes; i++) {
		ld_processes.path[i] = (char*)malloc(sizeof(char) * 100);
		ld_processes.path[i][0] = '\0';
		strcat(ld_processes.path[i], "input/proc/");
		char proc[100];
		char line[256];
		int ok;

		if (i == 0 && has_pending_line) {
			snprintf(line, sizeof(line), "%s", pending_line);
		} else {
			if (!read_nonempty_line(file, line, sizeof(line)))
				break;
		}
#ifdef MLQ_SCHED
		ok = parse_process_line(line, &ld_processes.start_time[i], proc, &ld_processes.prio[i]);
#else
		ok = parse_process_line(line, &ld_processes.start_time[i], proc, NULL);
#endif
		if (!ok) {
			printf("Cannot parse configure file at %s\n", path);
			exit(1);
		}
		strcat(ld_processes.path[i], proc);
	}
}

int main(int argc, char * argv[]) {
	/* Read config */
	if (argc != 2) {
		printf("Usage: os [path to configure file]\n");
		return 1;
	}
	char path[100];
	path[0] = '\0';
	strcat(path, "input/");
	strcat(path, argv[1]);
	read_config(path);

	pthread_t * cpu = (pthread_t*)malloc(num_cpus * sizeof(pthread_t));
	struct cpu_args * args =
		(struct cpu_args*)malloc(sizeof(struct cpu_args) * num_cpus);
	pthread_t ld;
	
	/* Init timer */
	int i;
	for (i = 0; i < num_cpus; i++) {
		args[i].timer_id = attach_event();
		args[i].id = i;
	}
	struct timer_id_t * ld_event = attach_event();
	start_timer();

#ifdef MM_PAGING
	/* Init all MEMPHY include 1 MEMRAM and n of MEMSWP */
	int rdmflag = 1; /* By default memphy is RANDOM ACCESS MEMORY */

	struct memphy_struct mram;
	struct memphy_struct mswp[PAGING_MAX_MMSWP];

	/* Create MEM RAM */
	init_memphy(&mram, memramsz, rdmflag);

        /* Create all MEM SWAP */ 
	int sit;
	for(sit = 0; sit < PAGING_MAX_MMSWP; sit++)
	       init_memphy(&mswp[sit], memswpsz[sit], rdmflag);

	/* In Paging mode, it needs passing the system mem to each PCB through loader*/
	struct mmpaging_ld_args *mm_ld_args = malloc(sizeof(struct mmpaging_ld_args));

	mm_ld_args->timer_id = ld_event;
	mm_ld_args->mram = (struct memphy_struct *) &mram;
	mm_ld_args->mswp = (struct memphy_struct**) &mswp;
	mm_ld_args->active_mswp = (struct memphy_struct *) &mswp[0];
        mm_ld_args->active_mswp_id = 0;


#endif

	/* Init scheduler */
	init_scheduler();

	/* Run CPU and loader */
#ifdef MM_PAGING
	pthread_create(&ld, NULL, ld_routine, (void*)mm_ld_args);
#else
	pthread_create(&ld, NULL, ld_routine, (void*)ld_event);
#endif
	for (i = 0; i < num_cpus; i++) {
		pthread_create(&cpu[i], NULL,
			cpu_routine, (void*)&args[i]);
	}

	/* Wait for CPU and loader finishing */
	for (i = 0; i < num_cpus; i++) {
		pthread_join(cpu[i], NULL);
	}
	pthread_join(ld, NULL);

	/* Stop timer */
	stop_timer();

	return 0;

}



