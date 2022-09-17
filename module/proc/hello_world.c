#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Rusmanov");
MODULE_DESCRIPTION("\"Hello World!\" module");
MODULE_VERSION("0.0.1");

#define PROCFS_MAX_SIZE 2048 
static struct proc_dir_entry *our_proc_file; 
static char procfs_buffer[PROCFS_MAX_SIZE]; 
static unsigned long procfs_buffer_size = 0; 
static int failes=0;

static int irq=20;
module_param(irq,int,0660);
static int mode=1;
module_param(mode,int,0660);

static ssize_t procfile_read(struct file *filePointer, char __user *buffer, size_t buffer_length, loff_t *offset) 
{ 
	char buf[PROCFS_MAX_SIZE];
	int len=0;
	printk( KERN_DEBUG "read handler\n");
	if(*offset > 0 || buffer_length < PROCFS_MAX_SIZE)
		return 0;
	len += sprintf(buf,"irq = %d\n",irq);
	len += sprintf(buf + len,"mode = %d\n",mode);
	
	if(copy_to_user(buffer,buf,len))
		return -EFAULT;
	*offset = len;
	return len; 
} 

static ssize_t procfile_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) 
{ 
	printk( KERN_DEBUG "write handler\n");
	int num,c,i,m;
	char buf[PROCFS_MAX_SIZE];
	if(*ppos > 0 || count > PROCFS_MAX_SIZE)
		return -EFAULT;
	if(copy_from_user(buf,ubuf,count))
		return -EFAULT;
	num = sscanf(buf,"%d %d",&i,&m);
	if(num != 2)
		return -EFAULT;
	irq = i; 
	mode = m;
	c = strlen(buf);
	*ppos = c;
	printk( KERN_DEBUG "write handler %d %d\n", i,m);
	return c;
}

struct proc_dir_entry *proc_file_entry;

static const struct proc_ops proc_file_fops = { 
	.proc_read  = procfile_read,
	.proc_write  = procfile_write,
};

static int __init hello_init(void)
{
	printk("############!\n");
	our_proc_file = proc_create("proc_file_name", 0644, NULL, &proc_file_fops);
	if(our_proc_file == NULL)
	{
		proc_remove(our_proc_file);
		printk(KERN_ERR, "Unable to register \"Hello world\" proc file\n");
		return -ENOMEM;
	}
	return 0;
}


static void __exit hello_exit(void)
{
	printk("Goodbye world!\n");
	proc_remove(our_proc_file);
}

module_init(hello_init);
module_exit(hello_exit);


