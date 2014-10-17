/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2000-2008 H. Peter Anvin - All Rights Reserved
 *   Copyright 2009 Intel Corporation; author: H. Peter Anvin
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * x86 MSR access device
 *
 * This device is accessed by lseek() to the appropriate register number
 * and then read/write in chunks of 8 bytes.  A larger size means multiple
 * reads or writes of the same register.
 *
 * This driver uses /dev/cpu/%d/msr_safe where %d is the minor number, and on
 * an SMP box will direct the access to CPU %d.
 */


#include <linux/module.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/smp.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>

#include <asm/processor.h>

#define MASK_RANGE(m,n) ((((uint32_t)1<<((m)-(n)+1))-1)<<(n))
#define MASK_VAL(x,m,n) (((uint32_t)(x)&MASK_RANGE((m),(n)))>>(n))

static struct class *msr_class;
static int majordev;

struct smsr_entry{
	loff_t 		reg;	// Almost all MSR addresses are 16-bit values,
				//   including all the ones we care about.
				//   Use 32 bits for compatibility.

	u32		write_mask_0;	// Prevent writing to reserved bits and
	u32		write_mask_1;	//   reading/writing sensitive bits of

	int		arch;		// Architecture number
	const char*	strReg;		// Address of msr in string format
	const char*	strName;	// Name of register
};

//-- Define all architectures and make a whitelist for each --- 
#define _USE_ARCH_062D 
#define _USE_ARCH_062A 
#define _USE_ARCH_063E 
#define _USE_ARCH_063C
#define _USE_ARCH_0645
#define _USE_ARCH_0646
#define _USE_ARCH_EMPTY
#include "msr-supplemental.h"
#undef _USE_ARCH_062D
#undef _USE_ARCH_062A
#undef _USE_ARCH_063E
#undef _USE_ARCH_063C
#undef _USE_ARCH_0645
#undef _USE_ARCH_0646
#undef _USE_ARCH_EMPTY

typedef enum smsr{
//Entry1
#define SMSR_ENTRY(a,b,c,d) a
ENTRY1
#undef SMSR_ENTRY

//Sandy Bridge
#define SMSR_ENTRY(a,b,c,d) p062D_## a
SMSR_062D
#undef SMSR_ENTRY

//Sandy Bridge
#define SMSR_ENTRY(a,b,c,d) p062A_## a
SMSR_062A
#undef SMSR_ENTRY

//Ivy Bridge
#define SMSR_ENTRY(a,b,c,d) p063E_## a
SMSR_063E
#undef SMSR_ENTRY

//Haswell
#define SMSR_ENTRY(a,b,c,d) p063C_## a
SMSR_063C
#undef SMSR_ENTRY

//Haswell
#define SMSR_ENTRY(a,b,c,d) p0645_## a
SMSR_0645
#undef SMSR_ENTRY

//Haswell
#define SMSR_ENTRY(a,b,c,d) p0646_## a
SMSR_0646
#undef SMSR_ENTRY

//Ending Entry
#define SMSR_ENTRY(a,b,c,d) a
ENTRY_END
#undef SMSR_ENTRY

} smsr_t;

//Empty set
#define SMSR_ENTRY(a,b,c,d) {b,c,d,0,#b,#a}
struct smsr_entry whitelist_EMPTY[] = { SMSR_EMPTY };
#undef SMSR_ENTRY

//Sandy Bridge
#define SMSR_ENTRY(a,b,c,d) {b,c,d,1,#b,#a}
struct smsr_entry whitelist_062D[] = { ENTRY1 SMSR_062D ENTRY_END };
#undef SMSR_ENTRY

//Sandy Bridge
#define SMSR_ENTRY(a,b,c,d) {b,c,d,2,#b,#a}
struct smsr_entry whitelist_062A[] = { ENTRY1 SMSR_062A ENTRY_END };
#undef SMSR_ENTRY

//Ivy Bridge
#define SMSR_ENTRY(a,b,c,d) {b,c,d,3,#b,#a}
struct smsr_entry whitelist_063E[] = { ENTRY1 SMSR_063E ENTRY_END };
#undef SMSR_ENTRY

//Haswell
#define SMSR_ENTRY(a,b,c,d) {b,c,d,4,#b,#a}
struct smsr_entry whitelist_063C[] = { ENTRY1 SMSR_063C ENTRY_END };
#undef SMSR_ENTRY

//Haswell
#define SMSR_ENTRY(a,b,c,d) {b,c,d,5,#b,#a}
struct smsr_entry whitelist_0645[] = { ENTRY1 SMSR_0645 ENTRY_END };
#undef SMSR_ENTRY

//Haswell
#define SMSR_ENTRY(a,b,c,d) {b,c,d,6,#b,#a}
struct smsr_entry whitelist_0646[] = { ENTRY1 SMSR_0646 ENTRY_END };
#undef SMSR_ENTRY

static int init=0;
static int arch=0;
struct smsr_entry *whitelist=NULL;

//-- End define architectures and setup whitelists ----------------------------------------------------------

//-- Begin setup for sysfs ----------------------------------------------------------------------------------
static char *Version = "1.3.0";

static ssize_t show_version(struct class *cls, char *buf)
{
	sprintf(buf, "%s\n", Version);
	return strlen(buf) + 1;
}

static ssize_t show_avail(struct class *cls, char *buf)
{
	smsr_t entry;
	for (entry = 0; entry < SMSR_LAST_ENTRY; entry++){
		if ( (whitelist[entry].reg != 0x000) && (whitelist[entry].arch == arch) ){
			strlcat(buf, whitelist[entry].strReg, PAGE_SIZE);
			strlcat(buf, " ", PAGE_SIZE);
			strlcat(buf, whitelist[entry].strName, PAGE_SIZE);
			strlcat(buf, "\n", PAGE_SIZE);
		}
	
	}
	
	return strlen(buf) + 1;
}

static struct class_attribute class_attr[] = { 
	__ATTR(version, 0644, show_version, NULL),
	__ATTR(avail, 0644, show_avail, NULL),
	__ATTR_NULL
};


static struct class smsr_class =
{
	.name = "smsr",
	.owner = THIS_MODULE,
	.class_attrs = class_attr
};

//-- End setup for sysfs ------------------------------------------------------------------------------------

static void 
get_cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx)
{
        asm volatile(
                "xchg %%ebx, %%edi\n\tcpuid\n\txchg %%ebx, %%edi"
                        :"=a" (*eax), "=D" (*ebx), "=c" (*ecx), "=d" (*edx)
                        :"a" (leaf)
                );  
}

static int
getArch (void)
{
	uint32_t regs[4];
	uint32_t cpu_type, cpu_model, cpu_family, cpu_stepping; 
	uint32_t cpu_family_extended, cpu_model_extended;
	uint32_t cpu_family_adjusted, cpu_model_adjusted;
	char string[80];
	const char *arches[8];
        int arrayLength=7;
	int i;

	get_cpuid( 1, &regs[0], &regs[1], &regs[2], &regs[3] );
	// EAX
	cpu_family 		= MASK_VAL(regs[0],11, 8);
       	cpu_family_extended 	= MASK_VAL(regs[0],27,20);
	cpu_model		= MASK_VAL(regs[0], 7, 4);
	cpu_model_extended	= MASK_VAL(regs[0],19,16);
	cpu_type		= MASK_VAL(regs[0],13,12);
	cpu_stepping		= MASK_VAL(regs[0], 3, 0);

	if(cpu_family == 0xF){
		cpu_family_adjusted = cpu_family + cpu_family_extended;
	}else{
		cpu_family_adjusted = cpu_family;
	}

	if(cpu_family == 0x6 || cpu_family == 0xF){
		cpu_model_adjusted = (cpu_model_extended << 4) + cpu_model;
	}else{
		cpu_model_adjusted = cpu_model;
	}
	
	sprintf(string, "%02x_%X", cpu_family_adjusted, cpu_model_adjusted);

	arches[0]= "";		//EMPTY
	arches[1]= "06_2D";	//Sandy Bridge
        arches[2]= "06_2A";	//Sandy Bridge
        arches[3]= "06_3E";	//Ivy Bridge
        arches[4]= "06_3C";	//Haswell
        arches[5]= "06_45";	//Haswell
        arches[6]= "06_46";	//Haswell
                                                                      
                                                                      
        for(i=1; i< arrayLength; i++)
        {
        	if(strncmp(string,arches[i], strlen(arches[i])) == 0)
        		return i;	//Matches
        }

	return 0;	//Does not match known architectures
}

u16 get_whitelist_entry(loff_t reg)
{
	smsr_t entry;

	for (entry = 0; entry < SMSR_LAST_ENTRY; entry++){
		if ( whitelist[entry].reg == reg && whitelist[entry].arch == arch ){
			return entry;
		}
	}
	return 0;
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
	// Force access through read and write only.
	(void)file;
	(void)ioc;
	(void)arg;
	return -ENOTTY;
}

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
{
        loff_t ret;
        struct inode *inode = file->f_mapping->host;

        mutex_lock(&inode->i_mutex);
        switch (orig) {
        case 0:
                file->f_pos = offset;
                ret = file->f_pos;
                break;
        case 1:
                file->f_pos += offset;
                ret = file->f_pos;
                break;
        default:
                ret = -EINVAL;
        }   
        mutex_unlock(&inode->i_mutex);
        return ret;
}


static ssize_t msr_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	smsr_t idx;
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = -EACCES; //Initialize to Permission Denied

	// "Count" doesn't have any meaning here, as we
	// never want to read more than one msr at at time.
	if (count != 8){
		return -EINVAL;	/* Invalid chunk size */
	}

	idx = get_whitelist_entry( reg );

	if(idx){
		err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
		if (!err){
			if (copy_to_user(tmp, &data, 8)) {
				err = -EFAULT;
			}
			else {
				err = 0; //Success
			}
		}
	}
	return err ? err : 8;
}

static ssize_t msr_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	const u32 __user *tmp = (const u32 __user *)buf;
	u32 data[2];
	smsr_t idx;
	loff_t reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = -EACCES;

	// "Count" doesn't have any meaning here, as we
	// never want to write more than one msr at at time.
	if (count != 8)
		return -EINVAL;	/* Invalid chunk size */

	idx = get_whitelist_entry( reg );

	if(idx){	
		
		// If the write masks are zero, don't bother writing.
		if((whitelist[idx].write_mask_0 | whitelist[idx].write_mask_1 )) {
			if (copy_from_user(&data, tmp, 8)) {
				err = -EFAULT;
			} else {
				data[0] &= whitelist[idx].write_mask_0;
				data[1] &= whitelist[idx].write_mask_1;
				err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
			}
		} else {
			err = -EROFS;	//Read only MSR
		}
	}
	return err ? err : 8;
}


static int msr_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;
	int ret = 0;

	cpu = iminor(file->f_path.dentry->d_inode);

	if (cpu >= nr_cpu_ids || !cpu_online(cpu)) {
		ret = -ENXIO;	/* No such CPU */
		goto out;
	}
	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR))
		ret = -EIO;	/* MSR not supported */
out:
	return ret;
}

/*
 * File operations we support
 */
static const struct file_operations msr_fops = {
	.owner = THIS_MODULE,
	.read = msr_read,
	.write = msr_write,
	.open = msr_open,
	.llseek = msr_seek,		// always returns -EINVAL
	.unlocked_ioctl = msr_ioctl,	// always returns -ENOTTY
	.compat_ioctl = msr_ioctl,	// always returns -ENOTTY
};

static int __cpuinit msr_device_create(int cpu)
{
	struct device *dev;

	dev = device_create(msr_class, NULL, MKDEV(majordev, cpu), NULL,
			    "msr_safe%d", cpu);
	return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
	device_destroy(msr_class, MKDEV(majordev, cpu));
}

static int __cpuinit msr_class_cpu_callback(struct notifier_block *nfb,
				unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int err = 0;

	switch (action) {
	case CPU_UP_PREPARE:
		err = msr_device_create(cpu);
		break;
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
	case CPU_DEAD:
		msr_device_destroy(cpu);
		break;
	}
	return notifier_from_errno(err);
}

static struct notifier_block __refdata msr_class_cpu_notifier = {
	.notifier_call = msr_class_cpu_callback,
};

static char *msr_devnode(struct device *dev, mode_t *mode)
{
	return kasprintf(GFP_KERNEL, "cpu/%u/msr_safe", MINOR(dev->devt));
}

static int __init msr_init(void)
{
	int i, err = 0;
	int status;
	i = 0;
	
	majordev = __register_chrdev(0, 0, NR_CPUS, "cpu/msr_safe", &msr_fops);
	if (majordev < 0) {
		printk(KERN_ERR "msr_safe: unable to register device number\n");
		err = -EBUSY;
		goto out;
	}
	msr_class = class_create(THIS_MODULE, "msr_safe");
	if (IS_ERR(msr_class)) {
		err = PTR_ERR(msr_class);
		goto out_chrdev;
	}
	msr_class->devnode = msr_devnode;
	for_each_online_cpu(i) {
		err = msr_device_create(i);
		if (err != 0)
			goto out_class;
	}
	register_hotcpu_notifier(&msr_class_cpu_notifier);

	err = 0;
	// Check architecture
	if(!init)
	{
		init=1;
		arch=getArch();
		switch(arch) {
		case 1:
			whitelist=whitelist_062D;
			break;
		case 2:
			whitelist=whitelist_062A;
			break;
		case 3:
			whitelist=whitelist_063E;
			break;
		case 4:
			whitelist=whitelist_063C;
			break;
		case 5:
			whitelist=whitelist_0645;
			break;
		case 6:
			whitelist=whitelist_0646;
			break;
		default:
			whitelist=whitelist_EMPTY;
			break;
		}
	}
	
	// Add smsr class
	status = class_register(&smsr_class);
	if (status < 0)
		printk("Registering smsr class failed\n");
	
	goto out;

out_class:
	i = 0;
	for_each_online_cpu(i)
		msr_device_destroy(i);
	class_destroy(msr_class);
out_chrdev:
	__unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr_safe");
out:
	return err;
}

static void __exit msr_exit(void)
{
	int cpu = 0;
	// Unregister smsr class
	class_unregister(&smsr_class);

	// Rest of the clean up
	for_each_online_cpu(cpu)
		msr_device_destroy(cpu);
	class_destroy(msr_class);
	__unregister_chrdev(majordev, 0, NR_CPUS, "cpu/msr_safe");
	unregister_hotcpu_notifier(&msr_class_cpu_notifier);
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("Kathleen Shoga <shoga1@llnl.gov>");
MODULE_DESCRIPTION("x86 sanitized MSR driver");
MODULE_LICENSE("GPL");
