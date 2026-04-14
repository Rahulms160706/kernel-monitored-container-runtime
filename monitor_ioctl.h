#ifndef MONITOR_IOCTL_H
#define MONITOR_IOCTL_H

#include <linux/ioctl.h>

#define REGISTER_PID _IOW('a', 'a', struct process_info *)

struct process_info {
    int pid;
    unsigned long soft_limit;
    unsigned long hard_limit;
};

#endif
