#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>				

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vlad Rusmanov");
MODULE_DESCRIPTION("\"DEV WRITE!\" module");
MODULE_VERSION("1.0.3");

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

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
	.release = device_release
};


int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	
	if (Major < 0) {		
		printk("Registering the character device failed with %d\n", Major);
		return Major;
	}
	
	printk("### MODULE %d HAVE BEEN LOADED\n", Major);
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
	printk("You open this %d device %d times to -> ", Major ,counter++);
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
	static int bytes_readed = 0;
	
	printk(" -> read some data\n");

	if (bytes_readed != 0)
	{
		bytes_readed = 0;
		printk(" -> But there is no data\n");
		return bytes_readed;
	}
		else
	{
		printk(" -> and there is one");
	}
	
	bytes_readed = simple_read_from_buffer(buffer, length, offset, msg_Ptr, BUF_LEN);
	
	printk(" -> data have been readed\n");
	return bytes_readed;
}


static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	
	int i;
	static int offset;
	printk(" -> to write some data\n");
	
	i = simple_write_to_buffer(msg+offset,len,off,buff,BUF_LEN);
	
	offset += i;
	msg_Ptr = msg;
		
	printk(" -> and data is sucessfuly writen\n");
	
	return i;
}



