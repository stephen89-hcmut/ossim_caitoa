
#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

#define MAX_QUEUE_SIZE 50

struct queue_t {
	struct pcb_t * proc[MAX_QUEUE_SIZE];
	int size;
};

void enqueue(struct queue_t * q, struct pcb_t * proc);

struct pcb_t * dequeue(struct queue_t * q);

struct pcb_t * find_proc_by_pid(struct krnl_t *krnl, uint32_t pid);

int empty(struct queue_t * q);

#endif

