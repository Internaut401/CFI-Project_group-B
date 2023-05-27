#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mydriver"
#define DRIVER_CLASS "mydriverclass"

static int major = -1;
static struct cdev my_cdev;
static int secret_key = 0x41414141;
static dev_t dev;
static struct class *my_class;
static int d;

static int mydriver_open(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t mydriver_read(struct file *file, char __user *user, size_t l, loff_t *off)
{
    return 0;
}

static ssize_t mydriver_write (struct file *file, const char __user *user, size_t l, loff_t *off)
{
    return 0;
}

static int mydriver_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long mydriver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int data;
    switch (cmd) {
    case 0:
        if (copy_from_user(&d, (int *) arg, sizeof(int))) {
            return -EFAULT;
        }
        printk("Value written: %d\n", data);
        break;
    case 1:
        if (copy_to_user((int *) arg, &d, sizeof(int))) {
            return -EFAULT;
        }
        printk("Value read: %d\n", d);
        break;
    default:
        return -ENOTTY;
    }

    return 0;
}

static struct file_operations mydriver_fops = {
    .owner = THIS_MODULE,
    .open = mydriver_open,
    .read = mydriver_read,
    .write = mydriver_write,
    .release = mydriver_release,
    .unlocked_ioctl = mydriver_ioctl,
};

static int __init mydriver_init(void)
{
    int err;

    /* Allocate a device number */
    err = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (err) {
        pr_err("Failed to register device\n");
        return err;
    }

    major = MAJOR(dev);

    /* Create device class */
    my_class = class_create(THIS_MODULE, DRIVER_CLASS);

    /* create device file */
    device_create(my_class, NULL, dev, NULL, DEVICE_NAME);

    cdev_init(&my_cdev, &mydriver_fops);
    //my_cdev.owner = THIS_MODULE;
    //my_cdev.ops = &mydriver_fops;
        unregister_chrdev_region(dev, 1);

    err = cdev_add(&my_cdev, dev, 1);

    if (err) {
        device_destroy(my_class, dev);
        class_destroy(my_class);
        unregister_chrdev_region(dev, 1);
        pr_err("Failed to add device\n");
        return err;
    }

    return 0;
}

static void __exit mydriver_exit(void)
{
    cdev_del(&my_cdev);
    device_destroy(my_class, dev);
    class_destroy(my_class);
    unregister_chrdev_region(dev, 1);
    printk("Goodbye kernel!\n");
}

module_init(mydriver_init);
module_exit(mydriver_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("MyDriver");
MODULE_LICENSE("GPL");
