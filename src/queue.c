#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"


int empty(struct queue_t *q)
{
        if (q == NULL)
                return 1;
        return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
        if (q == NULL || proc == NULL)
                return;

        if (q->size >= MAX_QUEUE_SIZE)
                return;

        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t *dequeue(struct queue_t *q)
{
        if (q == NULL || q->size == 0)
                return NULL;





        struct pcb_t *proc = q->proc[0];

        if (q->size > 1)
                memmove(&q->proc[0], &q->proc[1],
                        (q->size - 1) * sizeof(struct pcb_t *));

        q->size--;
        q->proc[q->size] = NULL;

		return proc;
}

struct pcb_t *purgequeue(struct queue_t *q, struct pcb_t *proc)
{
        if (q == NULL || proc == NULL || q->size == 0)
                return NULL;

        int i;
        for (i = 0; i < q->size; i++) {
                if (q->proc[i] != proc)
                        continue;

                if (i < q->size - 1)
                        memmove(&q->proc[i], &q->proc[i + 1],
                                (q->size - i - 1) * sizeof(struct pcb_t *));

                q->size--;
                q->proc[q->size] = NULL;
                return proc;
        }

        return NULL;
}