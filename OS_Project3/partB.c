#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arda Kıray");

static int processid = -1;
struct pid *pidstruct;
struct task_struct *processList = &init_task; // used to loop around task_struct list
struct task_struct *fprocess; // founded process struct pointer
struct mm_struct *processmm = NULL;
struct vm_area_struct *vm;
int found = 0; // boolean value for the result of process search


module_param(processid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(myint, "Process ID");

static int __init partB_init(void)
{
  do{
    if(processList->pid == processid){
      printk(KERN_INFO "Process found. The name is : %s and pid : %d\n", processList->comm, processList->pid);
      fprocess = processList;
      found = 1;
    }
  }while((processList = next_task(processList)) != &init_task && found == 0);


  if(found == 0){
    printk(KERN_INFO "There is no such process with a pid: %d\n", processid);
    return 0;
  }
  
  // If the process is not a kernel process and therefore have an address space in the user context
  // mm_struct mm is assigned to processmm or else if it is an anonymous process the previous processes'
  // mm_struct which is active_mm is assigned to processmm.
  if(fprocess->mm != NULL){
    processmm = fprocess->mm;
  }
  else if(fprocess->active_mm != NULL){
    processmm = fprocess->active_mm;
  }
  else{
    printk(KERN_INFO "No virtual memory information is found...\n");
    return 0;
  }

   // FIRST LEVEL PAGE TABLE TRAVELSAL :

  pgd_t *pgd;

  struct vm_area_struct *pvm = processmm->mmap;
  
  unsigned long address = (pvm->vm_start);// >> 12) << 12; // process virtual address without offset

  // Below code traverses outmost page table by using its page number index pgd. If the pgd enrty
  // is empty or unsuitable loop returns for that entry by using none() and bad(). Else the entry
  // with corresponding parsed informations are printed.
  pgd = pgd_offset(fprocess->mm, address);
  int i;
  printk(KERN_INFO "First level table entries with their structures are below:\n");
  
  for(i = 0; i < 512; i++){
    
    if(processmm->pgd[i].pgd == 0){
      //printk(KERN_INFO "Entry no. %d : %lu\n", i, processmm->pgd[i].pgd);
      return 0;
    }

    if (pgd_none(*pgd) || unlikely(pgd_bad(*pgd)))
      return 0;

    if(processmm->pgd[i].pgd != 0){

      int a1 = (pgd[i].pgd)&(0x1); // Valid bit
      int a2 = (pgd[i].pgd >> 1)&(0x1); // Read/Write bit        
      int a3 = (pgd[i].pgd >> 2)&(0x1); // User/Supervisor bit
      int a4 = (pgd[i].pgd >> 3)&(0x1); // Page-level write through bit
      int a5 = (pgd[i].pgd >> 4)&(0x1); // Page-level cache disabled bit
      int a6 = (pgd[i].pgd >> 5)&(0x1); // Access bit
      int a7 = (pgd[i].pgd >> 6)&(0x1); // Ignored bit
      int a8 = (pgd[i].pgd >> 7)&(0x1); // Reserved bit
      int a9 = (pgd[i].pgd >> 8)&(0xF); // Ignored bits (4 bits)
      int a10 = (pgd[i].pgd >> 12)&(0xFFFFFF); // Physical frame base address (24 bits)
      int a11 = (pgd[i].pgd >> 36)&(0xFFFF); // Reserved bits (16 bits)
      int a12 = (pgd[i].pgd >> 52)&(0x7FF); // Ignored bits (11 bits)
      int a13 = (pgd[i].pgd >> 63)&(0x1); // Execute disabled bit

      printk(KERN_INFO "Entry no. %d : %lu\n", i, pgd[i].pgd);
      printk(KERN_INFO "   Valid bit: %lx\n", a1);
      printk(KERN_INFO "   Read/Write bit: %lx\n", a2);
      printk(KERN_INFO "   User/Supervisor bit: %lx\n", a3);
      printk(KERN_INFO "   Page-level write through bit: %lx\n", a4);
      printk(KERN_INFO "   Page-level cache disabled bit: %lx\n", a5);
      printk(KERN_INFO "   Access bit: %lx\n", a6);
      printk(KERN_INFO "   Ignored bit: %lx\n", a7);
      printk(KERN_INFO "   Reserved bit: %lx\n", a8);
      printk(KERN_INFO "   Ignored bits(4): %lx\n", a9);
      printk(KERN_INFO "   Physical frame base address(24): %lx\n", a10);
      printk(KERN_INFO "   Reserved bits(16): %lx\n", a11);
      printk(KERN_INFO "   Ignored bits(11): %lx\n", a12);
      printk(KERN_INFO "   Execute disabled bit: %lx\n", a13);
      printk(KERN_INFO "---------------------------------------------");
      
    }
  }

  // SECOND LEVEL PAGE TABLES TRAVELSAL :

  printk(KERN_INFO "Second level table entries with their structures are below:\n");

  p4d_t *p4d;
  pud_t *pud;
  p4d = p4d_offset(pgd, address);
  pud = pud_offset(p4d, address);
      
  for(i = 0; i < 262144; i++){
    
    if(pud[i].pud == 0){
      //printk(KERN_INFO "Entry no. %d : %lu\n", i, pud[i]);
      return 0;
    }

    if (pud_none(*pud) || unlikely(pud_bad(*pud)))
      return 0;

    if(pud[i].pud != 0){

      int a1 = (pud[i].pud)&(0x1); // Valid bit
      int a2 = (pud[i].pud >> 1)&(0x1); // Read/Write bit        
      int a3 = (pud[i].pud >> 2)&(0x1); // User/Supervisor bit
      int a4 = (pud[i].pud >> 3)&(0x1); // Page-level write through bit
      int a5 = (pud[i].pud >> 4)&(0x1); // Page-level cache disabled bit
      int a6 = (pud[i].pud >> 5)&(0x1); // Access bit
      int a7 = (pud[i].pud >> 6)&(0x1); // Ignored bit
      int a8 = (pud[i].pud >> 7)&(0x1); // Reserved bit
      int a9 = (pud[i].pud >> 8)&(0xF); // Ignored bits (4 bits)
      int a10 = (pud[i].pud >> 12)&(0xFFFFFF); // Physical frame base address (24 bits)
      int a11 = (pud[i].pud >> 36)&(0xFFFF); // Reserved bits (16 bits)
      int a12 = (pud[i].pud >> 52)&(0x7FF); // Ignored bits (11 bits)
      int a13 = (pud[i].pud >> 63)&(0x1); // Execute disabled bit

      printk(KERN_INFO "Entry no. %d : %lu\n", i, pud[i]);
      printk(KERN_INFO "   Valid bit: %lx\n", a1);
      printk(KERN_INFO "   Read/Write bit: %lx\n", a2);
      printk(KERN_INFO "   User/Supervisor bit: %lx\n", a3);
      printk(KERN_INFO "   Page-level write through bit: %lx\n", a4);
      printk(KERN_INFO "   Page-level cache disabled bit: %lx\n", a5);
      printk(KERN_INFO "   Access bit: %lx\n", a6);
      printk(KERN_INFO "   Ignored bit: %lx\n", a7);
      printk(KERN_INFO "   Reserved bit: %lx\n", a8);
      printk(KERN_INFO "   Ignored bits(4): %lx\n", a9);
      printk(KERN_INFO "   Physical frame base address(24): %lx\n", a10);
      printk(KERN_INFO "   Reserved bits(16): %lx\n", a11);
      printk(KERN_INFO "   Ignored bits(11): %lx\n", a12);
      printk(KERN_INFO "   Execute disabled bit: %lx\n", a13);
      printk(KERN_INFO "---------------------------------------------------\n");
    }
  }


  // THIRD LEVEL PAGE TABLES TRAVELSAL :

    printk(KERN_INFO "Third level table entries with their structures are below:\n");

    pmd_t *pmd;
    pmd = pmd_offset(pud, address);
    
    for(i = 0; i < 134217728; i++){
      if(pmd[i].pmd == 0){
	//printk(KERN_INFO "Entry no. %d : %lu\n", i, pmd[i]);
	return 0;
      }

      if (pmd_none(*pmd) || unlikely(pmd_bad(*pmd)))
	return 0;

      if(pmd[i].pmd != 0){

	int a1 = (pmd[i].pmd)&(0x1); // Valid bit
	int a2 = (pmd[i].pmd >> 1)&(0x1); // Read/Write bit        
	int a3 = (pmd[i].pmd >> 2)&(0x1); // User/Supervisor bit
	int a4 = (pmd[i].pmd >> 3)&(0x1); // Page-level write through bit
	int a5 = (pmd[i].pmd >> 4)&(0x1); // Page-level cache disabled bit
	int a6 = (pmd[i].pmd >> 5)&(0x1); // Access bit
	int a7 = (pmd[i].pmd >> 6)&(0x1); // Ignored bit
	int a8 = (pmd[i].pmd >> 7)&(0x1); // Reserved bit
	int a9 = (pmd[i].pmd >> 8)&(0xF); // Ignored bits (4 bits)
	int a10 = (pmd[i].pmd >> 12)&(0xFFFFFF); // Physical frame base address (24 bits)
	int a11 = (pmd[i].pmd >> 36)&(0xFFFF); // Reserved bits (16 bits)
	int a12 = (pmd[i].pmd >> 52)&(0x7FF); // Ignored bits (11 bits)
	int a13 = (pmd[i].pmd >> 63)&(0x1); // Execute disabled bit

	printk(KERN_INFO "Entry no. %d : %lu\n", i, pmd[i]);
	printk(KERN_INFO "   Valid bit: %lx\n", a1);
	printk(KERN_INFO "   Read/Write bit: %lx\n", a2);
	printk(KERN_INFO "   User/Supervisor bit: %lx\n", a3);
	printk(KERN_INFO "   Page-level write through bit: %lx\n", a4);
	printk(KERN_INFO "   Page-level cache disabled bit: %lx\n", a5);
	printk(KERN_INFO "   Access bit: %lx\n", a6);
	printk(KERN_INFO "   Ignored bit: %lx\n", a7);
	printk(KERN_INFO "   Reserved bit: %lx\n", a8);
	printk(KERN_INFO "   Ignored bits(4): %lx\n", a9);
	printk(KERN_INFO "   Physical frame base address(24): %lx\n", a10);
	printk(KERN_INFO "   Reserved bits(16): %lx\n", a11);
	printk(KERN_INFO "   Ignored bits(11): %lx\n", a12);
	printk(KERN_INFO "   Execute disabled bit: %lx\n", a13);
	printk(KERN_INFO "---------------------------------------------------\n");
      
      }
    }

   // FORTH LEVEL PAGE TABLES TRAVELSAL :

    printk(KERN_INFO "Forth level table entries with their structures are below:\n");

    pte_t *pte;
    pte = pte_offset_kernel(pmd, address);
	
    for(i = 0; i < 68719476736; i++){
      if(pte[i].pte == 0){
	//printk(KERN_INFO "Entry no. %d : %lu\n", i, pte[i]);
	return 0;
      }

      if (pte_none(*pte) || unlikely(pte_accessible(processmm, *pte)))
	return 0;

      if(pte[i].pte != 0){

	int a1 = (pte[i].pte)&(0x1); // Valid bit
	int a2 = (pte[i].pte >> 1)&(0x1); // Read/Write bit        
	int a3 = (pte[i].pte >> 2)&(0x1); // User/Supervisor bit
	int a4 = (pte[i].pte >> 3)&(0x1); // Page-level write through bit
	int a5 = (pte[i].pte >> 4)&(0x1); // Page-level cache disabled bit
	int a6 = (pte[i].pte >> 5)&(0x1); // Access bit
	int a7 = (pte[i].pte >> 6)&(0x1); // Ignored bit
	int a8 = (pte[i].pte >> 7)&(0x1); // Reserved bit
	int a9 = (pte[i].pte >> 8)&(0xF); // Ignored bits (4 bits)
	int a10 = (pte[i].pte >> 12)&(0xFFFFFF); // Physical frame base address (24 bits)
	int a11 = (pte[i].pte >> 36)&(0xFFFF); // Reserved bits (16 bits)
	int a12 = (pte[i].pte >> 52)&(0x7FF); // Ignored bits (11 bits)
	int a13 = (pte[i].pte >> 63)&(0x1); // Execute disabled bit

	printk(KERN_INFO "Entry no. %d : %lu\n", i, pte[i]);
	printk(KERN_INFO "   Valid bit: %lx\n", a1);
	printk(KERN_INFO "   Read/Write bit: %lx\n", a2);
	printk(KERN_INFO "   User/Supervisor bit: %lx\n", a3);
	printk(KERN_INFO "   Page-level write through bit: %lx\n", a4);
	printk(KERN_INFO "   Page-level cache disabled bit: %lx\n", a5);
	printk(KERN_INFO "   Access bit: %lx\n", a6);
	printk(KERN_INFO "   Ignored bit: %lx\n", a7);
	printk(KERN_INFO "   Reserved bit: %lx\n", a8);
	printk(KERN_INFO "   Ignored bits(4): %lx\n", a9);
	printk(KERN_INFO "   Physical frame base address(24): %lx\n", a10);
	printk(KERN_INFO "   Reserved bits(16): %lx\n", a11);
	printk(KERN_INFO "   Ignored bits(11): %lx\n", a12);
	printk(KERN_INFO "   Execute disabled bit: %lx\n", a13);
	printk(KERN_INFO "---------------------------------------------------\n");
      
      }
    }

  return 0; 
}

static void __exit partB_exit(void)
{
  printk(KERN_INFO "Module is removed succesfully...\n");
}

module_init(partB_init);
module_exit(partB_exit);

