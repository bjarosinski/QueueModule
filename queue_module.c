#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("FIFO/LIFO Queue");
MODULE_AUTHOR("b.jarosinski");

#define MAX_QUEUE_SIZE 1024
//typedef Message char[100];

enum queue_type
{
	FIFO = 0,
	LIFO
};

enum queue_type q_type = FIFO;
static char queue[1024][100];
static short readPos = 0;
static int times = 0;
static int current_size = 0;


static int dev_open(struct inode*, struct file*);
static int dev_rls (struct inode*, struct file*);
static ssize_t dev_read (struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops =
{
		.read = dev_read,
		.open = dev_open,
		.write = dev_write,
		.release = dev_rls
};

int init_module(void)
{
	int t = register_chrdev(89, "myDev", &fops);
	if(t<0)
	{
		printk(KERN_ALERT "Device registration failed..\n");
	}
	else
	{
		printk(KERN_ALERT "Device registered...\n");
	}
	return t;
}

void cleanup_module(void)
{
	unregister_chrdev(89,"myDev");
}

static int dev_open(struct inode* inod, struct file *fil)
{
	times++;
	printk(KERN_ALERT"Device opened %d times\n", times);
	return 0;
}


static ssize_t dev_read(struct file *filp, char *buff, size_t count, loff_t *offset)
{
	//short count = 0;
	int queueElementCounter;
	int len = 0;
	if(current_size > 0)
	{
		printk(KERN_INFO "%s",queue[current_size-1]);
	     len = count >= strlen(queue[current_size-1]) ? strlen(queue[current_size-1]) : count;

	    if (*offset >= strlen(queue[current_size-1]))
	        return 0;

	    if (copy_to_user(buff,queue[current_size-1],len))
	        return -EFAULT;

		for(queueElementCounter = 0 ; queueElementCounter < current_size - 1; queueElementCounter++)
		{
			*queue[queueElementCounter] = *queue[queueElementCounter+1];
		}
		printk(KERN_INFO "After %s",queue[current_size-1]);
		current_size--;
	}
	return len;
}

static int dev_rls (struct inode* inod, struct file* fil)
{
	printk(KERN_ALERT "Device closed\n");
	return 0;
}

static ssize_t dev_write(struct file *filp, const char*buff, size_t len, loff_t *off)
{
	short ind = 0;
	short count = 0;
	readPos = 0;
	unsigned long ret;
	printk(KERN_INFO "Inside write \n");
	if (count > sizeof(queue[current_size]) - 1)
	    return -EINVAL;
	ret = copy_from_user(queue[current_size],buff,len);
	if (ret)
	    return -EFAULT;
	queue[current_size][len] = '\0';
	printk(KERN_INFO "After write: %s",queue[current_size]);
	current_size++;
	return len;
}
