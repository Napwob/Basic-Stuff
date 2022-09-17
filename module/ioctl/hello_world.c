#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>				
#include "chardev.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Rusmanov");
MODULE_DESCRIPTION("\"DEV WRITE!\" module");
MODULE_VERSION("0.0.3");

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);

#define SUCCESS 0
#define DEVICE_NAME "chardev"	 
#define BUF_LEN 80										 

static int Major;						 
static int Device_Open = 0;	 
															 
static char msg[BUF_LEN];		 
static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.unlocked_ioctl = device_ioctl,
	.release = device_release,
};


int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	
	if (Major < 0) {		
		printk("Registering the character device failed with %d\n", Major);
		return Major;
	}
	
	printk("### MODULE %d HAVE BEEN LOADED\n\n", Major);
	return 0;
}

void cleanup_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME);
	printk("### MODULE %d HAVE BEEN UNLOADED\n", Major);
}


static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 1;
	if (Device_Open)
		return -EBUSY;
	Device_Open++;
	printk("You open this %d device %d times to    ", Major ,counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
	
	return SUCCESS;
}


static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;	
	printk("You close this %d device\n\n", Major);							
	module_put(THIS_MODULE);

	return 0;
}


static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{

	int bytes_readed = 0;
	
	printk("    read some data\n");

	if (*msg_Ptr == 0)
	{
		printk("    But there is no data\n");
		return 0;
	}
		else
	{
		printk("    and there is one");
	}

	
	while (length && *msg_Ptr) 
	{
		put_user(*(msg_Ptr++), buffer++);
		length--;
		bytes_readed++;
	}
	printk("    data have been readed\n");
	return bytes_readed;
}


static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	
	int i;
	static int offset=0;
	printk("    write some data\n");
	
	for (i = 0; i < len && i < BUF_LEN; i++)
	{
		get_user(msg[i+offset], buff + i);
	}
	
	offset = i + offset;
	msg_Ptr = msg;
		
	printk("    and data is sucessfuly writen\n");
	
	return i;
}

long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	int i;
	char *temp;
	char ch;
	
	printk("    call ioctl part for");
	
	switch (ioctl_num) 
	{
		case IOCTL_SET_MSG:
			
			printk("        write some data by ioctl\n");
			
			temp = (char *)ioctl_param;

			get_user(ch, temp);
			for (i = 0; ch && i < BUF_LEN; i++, temp++)
				get_user(ch, temp);
			printk("        Lets call device write\n");
			device_write(file, (char *)ioctl_param, i, 0);
			break;

		case IOCTL_GET_MSG:
			
			printk("        read some data by ioctl\n");
			
			printk("        Lets call device read\n");	
			i = device_read(file, (char *)ioctl_param, 99, 0);

			put_user('\0', (char *)ioctl_param + i);
			break;
		
		case IOCTL_GET_NTH_BYTE:
			printk("        get some byte of message ioctl\n");
			return msg[ioctl_param];
			break;
	}

	return SUCCESS;
}


