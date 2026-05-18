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
	*pgd = (addr&PAGING64_ADDR_PGD_MASK)>>PAGING64_ADDR_PGD_LOBIT;
	*p4d = (addr&PAGING64_ADDR_P4D_MASK)>>PAGING64_ADDR_P4D_LOBIT;
	*pud = (addr&PAGING64_ADDR_PUD_MASK)>>PAGING64_ADDR_PUD_LOBIT;
	*pmd = (addr&PAGING64_ADDR_PMD_MASK)>>PAGING64_ADDR_PMD_LOBIT;
	*pt = (addr&PAGING64_ADDR_PT_MASK)>>PAGING64_ADDR_PT_LOBIT;

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
