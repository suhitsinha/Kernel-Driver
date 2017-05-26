#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/pgtable.h>

//extern mm_struct init_mm;

struct sample
{
	int rollno;
	int marks;
};

int sampleModuleStart(void)
{


	pgd_t *PGDir;
	pud_t *PUDir;
	pmd_t *PMDir;
	pte_t *PTEnt ;
	//unsigned long phyadd;
	unsigned long ptrval, off;
	struct sample *ptr = (struct sample *) vmalloc(sizeof(struct sample));

	ptr->rollno = 1530;
	ptr->marks = 50;
	ptrval = (unsigned long) ptr;
	printk("Address to structure: %lu\n", ptrval);
	
	printk("Structure: Virtual address : %lx and Physical address : %lx\n", ptrval, __pa(ptr));
	//printk("Structure: Virtual address : %lx and Physical address : %lx\n", ptrval, __(ptr));
	
	PGDir = pgd_offset(current->mm, ptrval);
	if (pgd_none(*PGDir))
	{
		printk("PGD not found\n");
		return 0;
	}
	PUDir = pud_offset(PGDir, ptrval);
	if (pud_none(*PUDir))
	{	
		printk("PUD not found\n");
		return 0;
	}
	
	PMDir = pmd_offset(PUDir, ptrval);
	if (pmd_none(*PMDir))
	{
		printk("PGD not found\n");
		return 0;
	}

	PTEnt = pte_offset_map(PMDir, ptrval);

	if(!pte_present(*PTEnt))
	{
		printk("PTE not found\n");
		return 0;
	}

	off = (ptrval & (PAGE_SIZE - 1));

	printk("Form software walk: Structure Virtual address : %lx and Physical address : %lx\n", ptrval, ((pte_pfn(*PTEnt) << PAGE_SHIFT) + off));
	
        return 0;
}
void sampleModuleEnd(void)
{
	printk(KERN_EMERG "********Exiting Module*********\n");
}


module_init(sampleModuleStart);
module_exit(sampleModuleEnd);

MODULE_LICENSE("Suhit");
