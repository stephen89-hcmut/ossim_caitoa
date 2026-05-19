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
