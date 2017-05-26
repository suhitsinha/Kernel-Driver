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
#include <asm/pgtable.h>

long int totalrss=0, totalva=0,pre=0, mapped=0, notpre=0, pg=0;
pgd_t *PGDir;
pud_t *PUDir;
pmd_t *PMDir;
pte_t *PTEnt ;

static int virphyprint(struct mm_struct *mm, unsigned long ptrval)
{
	unsigned long /*phyadd, */off;
	PGDir = pgd_offset(mm, ptrval);
	if (pgd_none(*PGDir))
	{
		return 0;
	}
	PUDir = pud_offset(PGDir, ptrval);
	if (pud_none(*PUDir))
	{
		return 0;
	}
	
	PMDir = pmd_offset(PUDir, ptrval);
	if (pmd_none(*PMDir))
	{
		return 0;
	}

	PTEnt = pte_offset_map(PMDir, ptrval);

	if (PTEnt == NULL)
	{
		return 0;
	}

	if(pte_present(*PTEnt))
	{
		off = (ptrval & (PAGE_SIZE - 1));
		printk("Form software walk: Structure Virtual address : %lx and Physical address : %lx\n", ptrval, ((pte_pfn(*PTEnt) << PAGE_SHIFT) + off));
	}
	return 1;

}

static int virphy(struct mm_struct *mm, unsigned long ptrval)
{
	unsigned long /*phyadd, */off;
	PGDir = pgd_offset(mm, ptrval);
	if (pgd_none(*PGDir))
	{
		return 0;
	}
	PUDir = pud_offset(PGDir, ptrval);
	if (pud_none(*PUDir))
	{	
		return 0;
	}
	
	PMDir = pmd_offset(PUDir, ptrval);
	if (pmd_none(*PMDir))
	{
		
		return 0;
	}

	PTEnt = pte_offset_map(PMDir, ptrval);

	if (PTEnt == NULL)
	{
		return 0;
	}

	if(!pte_present(*PTEnt))
	{
		mapped++;

	}

	else
	{
		off = (ptrval & (PAGE_SIZE - 1));
		pre++;
		totalrss++;
	}
	return 1;

}



int sampleModuleStart(void)
{
	unsigned long vaddr;
	int i;

	unsigned long vastart[8] = {0xffff880000000000, 0xffffc90000000000, 0xffffea0000000000, 0xffffec0000000000, 0xffffff0000000000, 0xffffffff80000000, 0xffffffffa0000000, 0xffffffffff600000};
	unsigned long vaend[8] = {0xffffc7ffffffffff, 0xffffe8ffffffffff, 0xffffeaffffffffff, 0xfffffc0000000000, 0xffffff7fffffffff, 0xffffffffa0000000, 0xffffffffff5fffff, 0xffffffffffdfffff};

	vaddr = vastart[0];

	while(vaddr < vaend[0])
	{
		if(!virphyprint(current->mm,vaddr))
			break;
			vaddr = vaddr + PAGE_SIZE;
	}


	for(i=0;i<8;i++)
	{
		vaddr = vastart[i];
		pre=0;
		notpre=0;
		mapped = 0;
		pg=0;
		while(vaddr < vaend[i])
		{
			if(!virphy(current->mm,vaddr))
				break;
			vaddr = vaddr + PAGE_SIZE;
		}
		totalva = totalva + ((vaend[i] - vastart[i])/PAGE_SIZE);
		pg = (vaend[i] - vastart[i])/PAGE_SIZE;
		printk("==================================================================================\n");
		printk("Mapping for virtual address: %lx - %lx\n", vastart[i], vaend[i]);
		printk("Mapped and present pages: %ld\n", pre);
		printk("Mapped but not present pages: %ld\n", mapped);
		printk("Not mapped pages: %ld\n", pg-pre-mapped);
		printk("Total pages in the virtual area: %ld\n", pg);
		printk("==================================================================================\n");
	}

	printk("Total RSS %ldK\n", totalrss*4);
	printk("Total Virtual address size %ldK\n", totalva*4);

	return 0;
}
void sampleModuleEnd(void)
{
	printk(KERN_EMERG "********Exiting Module*********\n");
}


module_init(sampleModuleStart);
module_exit(sampleModuleEnd);


MODULE_LICENSE("GPL");
