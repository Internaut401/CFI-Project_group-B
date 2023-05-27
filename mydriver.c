#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "driver/QARMA64.h"

#define DEVICE_NAME "mydriver"
#define DRIVER_CLASS "mydriverclass"

static int major = -1;
static struct cdev my_cdev;
static void *secret_key = 0xffffffff00000000;
static text_t tmp = 0;

text_t plaintext = 0xfb623599da6e8127;
key_t w0 = 0x84be85ce9804e94b;
key_t k0 = 0xec2802d4e0a488e9;
tweak_t tweak = 0x477d469dec0b8762;


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

    void *data;
    switch (cmd) {
    case 0:
        //encrypt pointer
        data = (void *)arg;
        if (copy_from_user(&tmp, data, sizeof(uintptr_t))) {
            return -EFAULT;
        }
        tmp = qarma64_enc(tmp, tweak, w0, k0, 5);
        if (copy_to_user(data, &tmp, sizeof(uintptr_t))) {
            return -EFAULT;
        }
        tmp = 0;
        break;

    case 1:
        data = (void *)arg;
        if (copy_from_user(&tmp, data, sizeof(uintptr_t))) {
            return -EFAULT;
        }
        tmp = qarma64_dec(tmp, tweak, w0, k0, 5);
        if (copy_to_user(data, &tmp, sizeof(uintptr_t))) {
            return -EFAULT;
        }
        tmp = 0;
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

struct class *my_class;

static int mydriver_init(void)
{
    dev_t dev;
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
    my_cdev.owner = THIS_MODULE;
    my_cdev.ops = &mydriver_fops;

    err = cdev_add(&my_cdev, dev, 1);

    if (err) {
        unregister_chrdev_region(dev, 1);
        pr_err("Failed to add device\n");
        return err;
    }

    return 0;
}

static void mydriver_exit(void)
{
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
}

module_init(mydriver_init);
module_exit(mydriver_exit);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("MyDriver");
MODULE_LICENSE("GPL");
