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
static dev_t dev;
static struct class *my_class;

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

static ssize_t mydriver_write(struct file *file, const char __user *user, size_t l, loff_t *off)
{
    return 0;
}

static int mydriver_release(struct inode *inode, struct file *file)
{
    return 0;
}

static long encrypt(uintptr_t __user *ret_addr_ptr) {
    uintptr_t ret_addr;
    if (copy_from_user(&ret_addr, ret_addr_ptr, sizeof(uintptr_t))) {
        return -EFAULT;
    }
    printk("Extracted return address 0x%016lx\n", ret_addr);
    // Encrypt
    ret_addr = qarma64_enc(ret_addr, tweak, w0, k0, 5);
    
    printk("Produced encrypted return address 0x%016lx\n", ret_addr);
    // Overwrite
    if (copy_to_user(ret_addr_ptr, &ret_addr, sizeof(uintptr_t))) {
        return -EFAULT;
    }
    return 0;
}

static long check(uintptr_t __user *ret_addr_ptr) {
    uintptr_t ret_addr;
    if (copy_from_user(&ret_addr, ret_addr_ptr, sizeof(uintptr_t))) {
        return -EFAULT;
    }
    printk("Extracted return address 0x%016lx\n", ret_addr);
    // Check signature
    ret_addr = qarma64_dec(ret_addr, tweak, w0, k0, 5);
    printk("Produced decrypted return address 0x%016lx\n", ret_addr);
    // Overwrite
    if (copy_to_user(ret_addr_ptr, &ret_addr, sizeof(uintptr_t))) {
        return -EFAULT;
    }
    return 0;
}

static long gen_key(void) {
    secret_key = 0xffffffff00000000;
    return 0;
}

static long mydriver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    switch (cmd) {
    case 0:
        ret = encrypt((uintptr_t *) arg);
        break;
    case 1:
        ret = check((uintptr_t *) arg);
        break;
    case 2:
        ret = gen_key();
    default:
        return -ENOTTY;
    }

    return ret;
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
