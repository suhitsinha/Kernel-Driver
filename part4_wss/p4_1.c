#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <asm/current.h>
#include <asm/pgtable.h>

extern void (*funcProcessPageHook) (int, unsigned long, struct mm_struct *);

static int upidval = 0;
int wss=0;

module_param(upidval, int, S_IRUSR);

void funcProcessPageHookModule(int upid, unsigned long ptrval, struct mm_struct *mms)
{
	pgd_t *PGDir;
	pud_t *PUDir;
	pmd_t *PMDir;
	pte_t *PTEnt ;
	if(upidval != upid)
		return;

	PGDir = pgd_offset(mms, ptrval);
	if (pgd_none(*PGDir))
	{
		return;
	}
	PUDir = pud_offset(PGDir, ptrval);
	if (pud_none(*PUDir) || pud_bad(*PUDir))
	{	
		return;
	}
	
	PMDir = pmd_offset(PUDir, ptrval);
	if (pmd_none(*PMDir) || pmd_bad(*PMDir))
	{
		return;
	}

	PTEnt = pte_offset_map(PMDir, ptrval);

	if (PTEnt == NULL)
	{
		return;
	}

	if(!pte_present(*PTEnt))
	{
		return;
	}
	//printk("I am here...\n");
	if((pte_val(*PTEnt) & _PAGE_PROTNONE) && !(pte_val(*PTEnt) & _PAGE_PRESENT))
	{
		//(*PTEnt).pte = ((((*PTEnt).pte) & (~_PAGE_PROTNONE)) | _PAGE_PRESENT);
		(*PTEnt).pte = 	(pte_val(*PTEnt)& ~_PAGE_PROTNONE)| _PAGE_PRESENT;
		wss++;
	}
	//printk("I am here...\n");
	return;
}

void virphy(struct mm_struct *mm, unsigned long ptrval)
{
	pgd_t *PGDir;
	pud_t *PUDir;
	pmd_t *PMDir;
	pte_t *PTEnt ;

	PGDir = pgd_offset(mm, ptrval);
	if (pgd_none(*PGDir) || pgd_bad(*PGDir))
	{
		return;
	}
	PUDir = pud_offset(PGDir, ptrval);
	if (pud_none(*PUDir) || pud_bad(*PUDir))
	{	
		return;
	}
	
	PMDir = pmd_offset(PUDir, ptrval);
	if (pmd_none(*PMDir) || pmd_bad(*PMDir))
	{
		return;
	}

	PTEnt = pte_offset_map(PMDir, ptrval);

	if (PTEnt == NULL)
	{
		return;
	}

	if(!pte_present(*PTEnt))
	{
		return;
	}
	
	else
	{
		//printk("I am local123...\n");
		//((*PTEnt).pte) = ((((*PTEnt).pte) | _PAGE_PROTNONE) & (~_PAGE_PRESENT));
		(*PTEnt).pte = (pte_val(*PTEnt)| _PAGE_PROTNONE)& (~_PAGE_PRESENT);
	}
	//printk("I am local...\n");
	//pte_unmap(PTE);
	return; 

}

int sampleModuleStart(void)
{
	int i=0;
	struct mm_struct *mms;
	struct vm_area_struct *vms;
	struct task_struct *task, *t;
	unsigned long vaddr;
	while(i<10)
	{
		task=NULL;
    		for_each_process(t)
    		{
			if(t->pid == upidval)
			{
				task=t;
    				break;
			}
    		}
		if(task->mm || task == NULL)
		{
			mms = task ->mm;
			if(mms->mmap)
			{
				vms = mms -> mmap;
	
				while( vms != NULL )
				{
					vaddr = vms->vm_start;
					while(vaddr < vms->vm_end)
					{
						virphy(mms,vaddr);
						vaddr = vaddr + PAGE_SIZE;
					}
					vms = vms->vm_next;
				}
			}
			else
			{
				printk("mmap not present...\n");
				return 0;
			}
		
		}
		else
		{
			printk("mm not present...\n");
			return 0;
		}
	
		funcProcessPageHook = funcProcessPageHookModule;
		i++;
		msleep_interruptible(10000);

		funcProcessPageHook=NULL;
		printk("WSS value at iteration no:%d is %d\n", i,wss);
		wss=0;
	}
       	return 0;
}
void sampleModuleEnd(void)
{
	funcProcessPageHook = NULL;
	printk("Exiting module\n");
}


module_init(sampleModuleStart);
module_exit(sampleModuleEnd);

MODULE_LICENSE("GPL");
