#include <linux/mm.h>
#include <linux/kthread.h>
#include <linux/sched/signal.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include "monitor_ioctl.h"
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "container_monitor"

/* Stores the PID, soft and hard limit from user programs */
struct proc_entry {
    int pid;
    unsigned long soft;
    unsigned long hard;
    struct list_head list;
};

static LIST_HEAD(proc_list);
static DEFINE_MUTEX(proc_lock);

static struct task_struct *monitor_thread;

static int major;
static struct class *cls;

/* Called when device is opened */
static int dev_open(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

/* Called when device is closed */
static int dev_close(struct inode *i, struct file *f)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    struct process_info info;

    if (cmd == REGISTER_PID) {
	struct proc_entry *entry;

        if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
            return -EFAULT;


	entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
	    return -ENOMEM;

	entry->pid = info.pid;
	entry->soft = info.soft_limit;
	entry->hard = info.hard_limit;

	mutex_lock(&proc_lock);
	list_add(&entry->list, &proc_list);
	mutex_unlock(&proc_lock);

	printk(KERN_INFO "[MONITOR] Stored PID: %d (soft=%lu, hard=%lu)\n",entry->pid, entry->soft, entry->hard);
    }

    return 0;
}

static int monitor_fn(void *data)
{
    while (!kthread_should_stop()) {

        struct proc_entry *entry;

        mutex_lock(&proc_lock);

        list_for_each_entry(entry, &proc_list, list) {

            struct task_struct *task;
	    
	    rcu_read_lock();

            task = pid_task(find_vpid(entry->pid), PIDTYPE_PID);
	    unsigned long rss;

            if (!task || !task->mm)
                continue;

            if (task->mm) {
                rss = get_mm_rss(task->mm) << PAGE_SHIFT;

		printk(KERN_INFO "[MONITOR DEBUG] PID %d RSS: %lu\n", entry->pid, rss);

                if (rss > entry->soft)
                    printk(KERN_INFO "[MONITOR] Soft limit exceeded for PID %d (RSS=%lu)\n", entry->pid, rss);

                if (rss > entry->hard) {
                    printk(KERN_INFO "[MONITOR] Killing PID %d (hard limit exceeded)\n", entry->pid);
                    send_sig(SIGKILL, task, 0);
                }
            }
        }
	rcu_read_unlock();

        mutex_unlock(&proc_lock);

        ssleep(1);
    }

    return 0;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_close,
    .unlocked_ioctl = dev_ioctl,
};

static int __init monitor_init(void)
{
    monitor_thread = kthread_run(monitor_fn, NULL, "monitor_thread");
    major = register_chrdev(0, DEVICE_NAME, &fops);

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    printk(KERN_INFO "[MONITOR] Device initialized\n");
    return 0;
}

static void __exit monitor_exit(void)
{
    struct proc_entry *entry, *tmp;

    // Stop thread
    if (monitor_thread)
        kthread_stop(monitor_thread);

    mutex_lock(&proc_lock);

    list_for_each_entry_safe(entry, tmp, &proc_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }

    mutex_unlock(&proc_lock);

    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, DEVICE_NAME);

    printk(KERN_INFO "[MONITOR] Module unloaded and cleaned up\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
