
// A global memory driver as an example of char device drivers
// modified by apenper_ee@163.com

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/io.h>

// if LINUX_KERNEL_VERSION > 3.3.x, use the asm/switch_to.h to replace the asm/system.h file
#include <asm/system.h>
#include <asm/uaccess.h>

#define GLOBALFIFO_SIZE	0x1000
#define MEM_CLEAR	0x1
#define GLOBALFIFO_MAJOR	250

static int globalfifo_major = GLOBALFIFO_MAJOR;

struct globalfifo_dev {
	struct cdev cdev;
	unsigned int curr_len;
	unsigned char mem[GLOBALFIFO_SIZE];
	struct semaphore sem;
	wait_queue_head_t r_wait;
	wait_queue_head_t w_wait;
};

struct globalfifo_dev *globalfifo_devp;

// file_operations structure open function
int globalfifo_open(struct inode *inode, struct file *filp)
{
	filp->private_data = globalfifo_devp;

	return 0;
}

// realease function
int globalfifo_release(struct inode *inode, struct file *filp)
{
	return 0;
}

// ioctl function
static int globalfifo_ioctl(struct inode *inodep, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct globalfifo_dev *dev = filp->private_data;
	
	switch(cmd) {
	case MEM_CLEAR:
		if (down_interruptible(&dev->sem)) {
			printk(KERN_INFO "ioctl receive one sig.\n");
			return -ERESTARTSYS;
		}

		memset(dev->mem, 0, GLOBALFIFO_SIZE);
		printk(KERN_INFO"globalfifo is set to zero.\n");
	default:
		return -EINVAL;
	}

	up(&dev->sem);
	printk(KERN_INFO "hello.\n");
	return 0;
}

// read function
static ssize_t globalfifo_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0;
	struct globalfifo_dev *dev = filp->private_data;

	// define and init the wait_queue
	DECLARE_WAITQUEUE(wait, current);

	down(&dev->sem);
	add_wait_queue(&dev->r_wait, &wait);

	// wait_event_interruptible(dev->r_wait, dev->curr_len != 0);

	// wait curr_len is not 0
	while (dev->curr_len == 0) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto out;
		}

		__set_current_state(TASK_INTERRUPTIBLE);

		// release the semaphore is very important, otherwise dead lock
		up(&dev->sem);

		// give up the CPU
		schedule();
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			goto out2;
		}

		down(&dev->sem);
	}

	// copy to user space
	if (count > dev->curr_len)
		count = dev->curr_len;
	
	if (copy_to_user(buf, dev->mem, count)) {
		ret = -EFAULT;
		goto out;
	} else {
		memcpy(dev->mem, dev->mem + count, dev->curr_len - count);
		dev->curr_len -= count;
		printk(KERN_INFO "read %d byte(s), current_len:%d\n", count, dev->curr_len);

		wake_up_interruptible(&dev->w_wait);
		ret = count;
	}
	
	out: up(&dev->sem);
	// note: remove w_wait not r_wait
	out2: remove_wait_queue(&dev->w_wait, &wait);
	set_current_state(TASK_RUNNING);

	return ret;
}

// write function
static ssize_t globalfifo_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	int ret = 0;
	struct globalfifo_dev *dev = filp->private_data;
	
	DECLARE_WAITQUEUE(wait, current);

	down(&dev->sem);
	add_wait_queue(&dev->w_wait, &wait);

	// wait_event_interruptible(dev->w_wait, dev->curr_len != GLOBALFIFO_SIZE);

	// wait FIFO not full
	while (dev->curr_len == GLOBALFIFO_SIZE) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto out;
		}
		__set_current_state(TASK_INTERRUPTIBLE);

		// release the semaphore is very important, otherwise dead lock
		up(&dev->sem);

		schedule();
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			goto out2;
		}

		down(&dev->sem);
	}


	if (count > GLOBALFIFO_SIZE - dev->curr_len) {
		count = GLOBALFIFO_SIZE - dev->curr_len;
	}

	if (copy_from_user(dev->mem + dev->curr_len, buf, count)) {
		ret = -EFAULT;
		goto out;
	} else {
		dev->curr_len += count;
		printk(KERN_INFO "written %d byte(s), curr_len:%d\n", count, dev->curr_len);

		wake_up_interruptible(&dev->r_wait);

		ret = count;
	}

	out:up(&dev->sem);
	out2: remove_wait_queue(&dev->w_wait, &wait);
	set_current_state(TASK_RUNNING);

	return ret;
}

// llseek function
static loff_t globalfifo_llseek(struct file *filp, loff_t offset, int orig)
{
	loff_t ret = 0;
	switch (orig) {
	case 0:
		if (offset < 0) {
			ret = -EINVAL;
			break;
		}
		if ((unsigned int)offset > GLOBALFIFO_SIZE) {
			ret = -EINVAL;
			break;
		}
		filp->f_pos = (unsigned int)offset;
		ret = filp->f_pos;
		break;
	case 1:
		if ((filp->f_pos + offset) > GLOBALFIFO_SIZE) {
			ret = -EINVAL;
			break;
		}
		if ((filp->f_pos + offset) < 0) {
			ret = -EINVAL;
			break;
		}
		filp->f_pos += offset;
		ret = filp->f_pos;
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static unsigned int globalfifo_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	struct globalfifo_dev *dev = filp->private_data;

	down(&dev->sem);

	poll_wait(filp, &dev->r_wait, wait);
	poll_wait(filp, &dev->w_wait, wait);

	// fifo not empty
	if (dev->curr_len != 0)
		mask |= POLLIN | POLLRDNORM;

	// fifo not full
	if (dev->curr_len != GLOBALFIFO_SIZE)
		mask |= POLLOUT | POLLWRNORM;

	up(&dev->sem);

	return mask;
}

// file_operations structure define
static const struct file_operations globalfifo_fops = {
	.owner = THIS_MODULE,
	.llseek = globalfifo_llseek,
	.read = globalfifo_read,
	.write = globalfifo_write,
	// .unlocked_ioctl = globalfifo_ioctl,
	.ioctl = globalfifo_ioctl,
	.open = globalfifo_open,
	.release = globalfifo_release,
	.poll = globalfifo_poll,
};

// cdev config function
static void globalfifo_setup_cdev(struct globalfifo_dev *dev, int minor)
{
	int err, devno = MKDEV(globalfifo_major, minor);

	cdev_init(&dev->cdev, &globalfifo_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &globalfifo_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_INFO"Error %d adding LED%d.\n", err, minor);
}

// globalfifo init function
int globalfifo_init(void)
{
	int result;
	dev_t devno = MKDEV(globalfifo_major, 0);

	if (globalfifo_major) 
		result = register_chrdev_region(devno, 1, "globalfifo");
	else {
		result = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
		globalfifo_major = MAJOR(devno);
	}
	if (result < 0)
		return result;

	globalfifo_devp = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
	if (!globalfifo_devp) {
		result = -ENOMEM;
		goto fail_malloc;
	}

	memset(globalfifo_devp, 0, sizeof(struct globalfifo_dev));

	globalfifo_setup_cdev(globalfifo_devp, 0);

	// init the MUTEX
	// Linux version > 2.6.32后，init_MUTEX被废除了
	// init_MUTEX(&globalfifo_devp->sem);
	sema_init(&globalfifo_devp->sem, 1);

	// init the wait_queue_head
	init_waitqueue_head(&globalfifo_devp->r_wait);
	init_waitqueue_head(&globalfifo_devp->w_wait);

	return 0;

	fail_malloc: unregister_chrdev_region(devno, 1);

	return result;
}

// exit function
void globalfifo_exit(void)
{
	cdev_del(&globalfifo_devp->cdev);
	kfree(globalfifo_devp);
	unregister_chrdev_region(MKDEV(globalfifo_major, 0), 1);
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("apenper apenper_ee@163.com");

module_param(globalfifo_major, int, S_IRUGO);
module_init(globalfifo_init);
module_exit(globalfifo_exit);


