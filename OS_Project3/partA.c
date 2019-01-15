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

static int __init partA_init(void)
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

  // Below loop writes all virtual memory area start adress and size pairs
  vm = processmm->mmap;
  while(vm->vm_next != NULL){
    printk(KERN_INFO "VM area start: 0x%lx VM area size: %lu\n", vm->vm_start, (vm->vm_end - vm->vm_start));
    vm = vm->vm_next;
  }
  
  // Below code gives detailed virtual memory informations about the given process
  if(processmm != NULL){
    printk(KERN_INFO "Below are some virtual memory informations of process %s\n", fprocess->comm);

    printk(KERN_INFO "The start address of the code segment: 0x%lx\n", processmm->start_code);
    printk(KERN_INFO "The end address of the code segment: 0x%lx\n", processmm->end_code);
    printk(KERN_INFO "The size of the code segment: %lu\n", (processmm->end_code - processmm->start_code));

    printk(KERN_INFO "The start address of the data segment: 0x%lx\n", processmm->start_data);
    printk(KERN_INFO "The end address of the data segment: 0x%lx\n", processmm->end_data);
    printk(KERN_INFO "The size of the data segment: %lu\n", (processmm->end_data - processmm->start_data));

    // Stack is traced via finding virtual memory area of it because no stack_end entry
    // exists in mm_struct. The stack flag is VM_GROWSDOWN.
    struct vm_area_struct *vmarea = processmm->mmap;
    int terminate = 0;
    do{
      if((vmarea->vm_flags|VM_GROWSDOWN) == vmarea->vm_flags){
	printk(KERN_INFO "The start address of the stack segment: 0x%lx\n", vmarea->vm_start);
	printk(KERN_INFO "The end address of the stack segment: 0x%lx\n", vmarea->vm_end);
	printk(KERN_INFO "The size of the stack segment: %lu\n", (vmarea->vm_end - vmarea->vm_start));
	terminate = 1;
      }
      vmarea = vmarea->vm_next;
    }while(terminate == 0);
	
    printk(KERN_INFO "The start address of the heap segment: 0x%lx\n", processmm->start_brk);
    printk(KERN_INFO "The end address of the heap segment: 0x%lx\n", processmm->brk);
    printk(KERN_INFO "The size of the heap segment: %lu\n", (processmm->brk - processmm->start_brk));

    printk(KERN_INFO "The start address of the main arguments: 0x%lx\n", processmm->arg_start);
    printk(KERN_INFO "The end address of the main arguments: 0x%lx\n", processmm->arg_end);
    printk(KERN_INFO "The size of the main arguments: %lu\n", (processmm->arg_end - processmm->arg_start));

    printk(KERN_INFO "The start address of the enviroment variables: 0x%lx\n", processmm->env_start);
    printk(KERN_INFO "The end address of the enviroment variables: 0x%lx\n", processmm->env_end);
    printk(KERN_INFO "The size of the enviroment variables: %lu\n", (processmm->env_end - processmm->env_start));

    printk(KERN_INFO "The number of the used frames: %lu\n", processmm->hiwater_rss);
    printk(KERN_INFO "Total virtual memory used: %lu\n", processmm->total_vm);
    
  }

  return 0; 
}

static void __exit partA_exit(void)
{
  printk(KERN_INFO "Module is removed succesfully...\n");
}

module_init(partA_init);
module_exit(partA_exit);
