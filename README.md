# Kernel Monitored Container Runtime

## Overview

This project implements a lightweight container runtime from scratch using Linux system primitives. It integrates a kernel module to monitor memory usage of containerized processes and enforce limits in real time.

The system demonstrates how user-space and kernel-space components interact to provide process isolation and resource control.

---

## Key Features

- Container creation using `clone()` with:
  - PID namespace
  - UTS namespace
  - Mount namespace
- Filesystem isolation using `chroot`
- Supervisor-based architecture with UNIX socket IPC
- Logging system using producer–consumer model
- Kernel module for real-time memory monitoring
- Memory enforcement:
  - Soft limit → warning
  - Hard limit → process termination (`SIGKILL`)
- Direct user-kernel communication using `ioctl`

---

## Architecture

```
CLI → Supervisor → Container → Kernel Module → Monitor Thread → Enforcement
```

---

## Project Structure

```
engine.c # User-space runtime and supervisor
monitor.c # Kernel module (LKM)
monitor_ioctl.h # Shared definitions for ioctl
Makefile # Builds everything with single command
workloads/ # Test programs (memhog, cpu_stress)
rootfs-alpha/ # Container filesystem
rootfs-base/ # Base filesystem
logs/ # Container logs
```

---

## Build Instructions

```bash
make
```

This builds:

- `engine` (user-space runtime)
- `monitor.ko` (kernel module)
- workload programs inside `rootfs-alpha`

---

## Running the project

1. Load Kernel Module

```bash
sudo insmod monitor.ko
```

2. Start Supervisor

```bash
sudo ./engine supervisor ./rootfs-base
```

3. Run Container (Memory Test)

```bash 
sudo ./engine run alpha ./rootfs-alpha "/memhog" --soft-mib 20 --hard-mib 40
```

4. Check Kernel Logs

```bash 
sudo dmesg | grep MONITOR
```

---

## Workloads

- **memhog** → consumes memory to trigger limits
- **cpu_stress** → simulates CPU workload

## Key Concepts Used

- Linux namespaces (PID, UTS, mount)
- `clone()` system call
- `chroot()` filesystem isolation
- Inter-process communication (UNIX sockets)
- Kernel-user communication via `ioctl`
- Process tracking using `task_struct`
- Memory monitoring via `mm_struct`
- Signal handling (`SIGKILL`)
- Producer–consumer synchronization

## Important Note

Root filesystem directories (rootfs-alpha, rootfs-base) are environment-specific and may require setup using tools like debootstrap.

---

## Conclusion

This project demonstrates the integration of user-space container runtime design with kernel-level resource monitoring, showcasing how operating system concepts like namespaces, process management, and memory control work together in real-world systems.

---

## Authors

- Rakshita Muttur
- Rahul M S
