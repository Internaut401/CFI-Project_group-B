#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "driver/QARMA64.h"
#include <linux/fcntl.h>
#include <linux/random.h>

#define DEVICE_NAME "mydriver"
#define DRIVER_CLASS "mydriverclass"
#define SIG_MASK 0xFFFF000000000000

static int major = -1;
static struct cdev my_cdev;
static dev_t dev;
static struct class *my_class;

//keys used for QARMA encryption / decryption
qkey_t secret_key_1;
qkey_t secret_key_2;


// Driver open function
static int mydriver_open(struct inode *inode, struct file *file){

    return 0;
}


// Driver read function
static ssize_t mydriver_read(struct file *file, char __user *user, size_t l, loff_t *off){

    return 0;
}


// Driver write function
static ssize_t mydriver_write(struct file *file, const char __user *user, size_t l, loff_t *off){

    return 0;
}


// Driver release function
static int mydriver_release(struct inode *inode, struct file *file){

    return 0;
}


// ioctl encrypt function
static long encrypt(uintptr_t __user *ret_addr_ptr) {

    uintptr_t ret_addr;
    uintptr_t rbp = (uintptr_t) ret_addr_ptr - sizeof(uintptr_t);
    uintptr_t sig;

    // copy retrun address from userland
    if (copy_from_user(&ret_addr, ret_addr_ptr, sizeof(uintptr_t))) {
        return -EFAULT;
    }
    printk(KERN_DEBUG "cfi-pa: extracted return address 0x%016lx\n", ret_addr);

    // taking just the used lowest 6 Byte 
    ret_addr &= ~SIG_MASK;

    // encrypting using QARMA and apply SIG_MASK to take just the highest 2 Byte
    sig = qarma64_enc(ret_addr, rbp, secret_key_1, secret_key_2, 5) & SIG_MASK;

    // saving the QARMA masked output into the 2 highest Byte of return address
    ret_addr |= sig;
    printk(KERN_DEBUG "cfi-pa: produced signed return address 0x%016lx\n", ret_addr);

    // Overwrite the old return address in userland with the new one 
    printk(KERN_DEBUG "rbp extracted: 0x%016lx\n", rbp);
    if (copy_to_user(ret_addr_ptr, &ret_addr, sizeof(uintptr_t))) {
        return -EFAULT;
    }

    return 0;
}


// ioctl check function
static long check(uintptr_t __user *ret_addr_ptr) {

    uintptr_t ret_addr;
    uintptr_t ret_sig;
    uintptr_t rbp = (uintptr_t) ret_addr_ptr - sizeof(uintptr_t);
    uintptr_t exp_sig;

    // copy retrun address from userland 
    if (copy_from_user(&ret_addr, ret_addr_ptr, sizeof(uintptr_t))) {
        return -EFAULT;
    }
    
    // extract the effective return address from the lowest 6 Byte
    ret_sig = ret_addr & SIG_MASK;

    // extract the encryption signature from the highest 2 Byte
    ret_addr &= ~SIG_MASK;
    printk(KERN_DEBUG "cfi-pa: extracted return address 0x%016lx (signature 0x%04lx)\n", ret_addr, ret_sig >> 48);
   
    // encrypt the return address and calculate the new signature (highest 2 Byte)
    exp_sig = qarma64_enc(ret_addr, rbp, secret_key_1, secret_key_2 , 5) & SIG_MASK;
    printk(KERN_DEBUG "My secret key: 0x%016llx\n ", secret_key_1);

    // Check signature validity
    if (exp_sig != ret_sig) {
        ret_addr = (uintptr_t) NULL;
        printk(KERN_ERR "cfi-pa: invalid return address signature: expected 0x%04lx, received 0x%04lx\n", exp_sig >> 48, ret_sig >> 48);
    } else {
        printk(KERN_DEBUG "cfi-pa: produced signature 0x%04lx (signature ok)\n", ret_sig >> 48);
    }

    // Overwrite the old return address in userland with the new one
    if (copy_to_user(ret_addr_ptr, &ret_addr, sizeof(uintptr_t))) {
        return -EFAULT;
    }
    return 0;
}


// random keys generation function
static long gen_key(void) {

    get_random_bytes(&secret_key_1, sizeof(secret_key_1));
    get_random_bytes(&secret_key_2, sizeof(secret_key_2));
    
    return 0;
}


static long mydriver_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

    long ret = 0;

    switch (cmd) {
    case 0:
        ret = encrypt((uintptr_t *) arg);
        break;
    case 1:
        ret = check((uintptr_t *) arg);
        break;
    case 2:
        gen_key();
        break;
    default:
        return -ENOTTY;
    }

    return ret;
}


// File operation sturcture
static struct file_operations mydriver_fops = {
    .owner = THIS_MODULE,
    .open = mydriver_open,
    .read = mydriver_read,
    .write = mydriver_write,
    .release = mydriver_release,
    .unlocked_ioctl = mydriver_ioctl,
};


static int __init mydriver_init(void){

    int err;

    /* Allocate a device number */
    err = alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME);
    if (err) {
        pr_err("Failed to register device\n");
        return err;
    }

    // extract the major number
    major = MAJOR(dev);

    /* Create device class */
    my_class = class_create(THIS_MODULE, DRIVER_CLASS);

    /* Create device file */
    device_create(my_class, NULL, dev, NULL, DEVICE_NAME);

    /* Creating cdev structure */
    cdev_init(&my_cdev, &mydriver_fops);

    /* Adding character device to the system */
    err = cdev_add(&my_cdev, dev, 1);

    if (err) {
        // deallocate structures in case of error
        device_destroy(my_class, dev);
        class_destroy(my_class);
        unregister_chrdev_region(dev, 1);
        pr_err("Failed to add device\n");
        return err;
    }

    // generate random keys
    gen_key();
    
    return 0;
}


static void __exit mydriver_exit(void){
    // deallocate structures and delete device
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
