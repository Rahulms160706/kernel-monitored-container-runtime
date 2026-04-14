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

## Build, Load, and Run Instructions

### 1. Build the Project

```bash
make
```
This builds:

- `engine` (user-space runtime)
- `monitor.ko` (kernel module)
- workload programs inside `rootfs-alpha`

### 2. Load Kernel Module

```bash
sudo insmod monitor.ko
```

### 3. Verify Device Creation

```bash
ls -l /dev/container_monitor
```

### 4. Start Supervisor

```bash
sudo ./engine supervisor ./rootfs-base
```

### 5. Create Writable Container Filesystems

```bash 
cp -a ./rootfs-base ./rootfs-alpha
cp -a ./rootfs-base ./rootfs-beta
```

### 6. Run Container (Memory Test)

```bash 
sudo ./engine run alpha ./rootfs-alpha "/memhog" --soft-mib 48 --hard-mib 80
sudo ./engine run beta ./rootfs-beta "/memhog" --soft-mib 64 --hard-mib 96
```

### 7. List Running Containers

```bash
sudo ./engine ps
```

### 8. View Container Logs

```bash
sudo ./engine logs alpha
```

### 9. Stop Containers

```bash
sudo ./engine stop alpha
sudo ./engine stop beta
```

### 10. Check Kernel Logs

```bash 
sudo dmesg | grep MONITOR
```

### 11. Unload Kernel Module

```bash
sudo rmmod monitor
```

---

## Demo Screenshots

### 1. Multi-container supervision
*(Insert screenshot showing two containers running under the same supervisor)*

---

### 2. Metadata tracking
*(Insert output of `engine ps` showing container details)*

---

### 3. Bounded-buffer logging
*(Insert log output showing container logs being captured)*

---

### 4. CLI and IPC
*(Insert screenshot showing CLI command and supervisor response)*

---

### 5. Soft-limit warning
*(Insert `dmesg` output showing soft limit exceeded)*

---

### 6. Hard-limit enforcement
*(Insert `dmesg` output showing container killed after exceeding hard limit)*

---

### 7. Scheduling experiment
*(Insert output showing workload behavior differences)*

---

### 8. Clean teardown
*(Insert evidence showing no zombie processes after stopping containers)*

---

## Engineering Analysis

This project demonstrates several core operating system concepts by combining user-space container management with kernel-level monitoring.

### 1. Process Isolation using Namespaces

Linux namespaces allow processes to have isolated views of system resources. In this project:

- PID namespace ensures each container has its own process ID space
- UTS namespace allows separate hostnames per container
- Mount namespace isolates filesystem changes

This enables lightweight containerization without the overhead of virtual machines.

---

### 2. User-space and Kernel-space Interaction

The system uses `ioctl` to communicate between user-space (engine) and kernel-space (monitor module).

- The engine registers container PIDs with memory limits
- The kernel module stores and tracks these processes
- This separation ensures control logic remains in user-space while enforcement happens in kernel-space

---

### 3. Process Monitoring in the Kernel

The kernel module periodically inspects processes using internal kernel structures:

- `task_struct` represents a process
- `mm_struct` contains memory-related information

By accessing these, the module retrieves memory usage and compares it against defined limits.

---

### 4. Memory Enforcement Mechanism

Two levels of enforcement are implemented:

- Soft limit → generates a warning when exceeded
- Hard limit → terminates the process using `SIGKILL`

This mirrors real-world resource control systems where gradual enforcement is preferred before termination.

---

### 5. Logging and Synchronization

The system uses a producer–consumer model for logging:

- Container output acts as producer
- Logging thread acts as consumer
- A bounded buffer ensures safe and efficient data transfer

This prevents race conditions and ensures logs are captured reliably.

---

## Design Decisions and Tradeoffs

### 1. Namespace-based Isolation

- **Choice:** Used Linux namespaces (`CLONE_NEWPID`, `CLONE_NEWUTS`, `CLONE_NEWNS`) for container isolation
- **Tradeoff:** Lightweight and fast, but less secure than full virtualization
- **Justification:** Suitable for educational and lightweight container systems where performance is preferred over full isolation

---

### 2. Supervisor-based Architecture

- **Choice:** Central supervisor process to manage containers
- **Tradeoff:** Single point of control simplifies management but introduces dependency on supervisor availability
- **Justification:** Enables centralized control, logging, and coordination of multiple containers

---

### 3. UNIX Socket for IPC

- **Choice:** Used UNIX domain sockets for communication between CLI and supervisor
- **Tradeoff:** Limited to local communication, but faster and more secure than network sockets
- **Justification:** Ideal for local container management with minimal overhead

---

### 4. Kernel Module for Monitoring

- **Choice:** Implemented memory monitoring in kernel-space
- **Tradeoff:** Increased complexity and risk compared to user-space monitoring
- **Justification:** Kernel has direct access to process structures, enabling accurate and efficient enforcement

---

### 5. Signal-based Enforcement

- **Choice:** Used `SIGKILL` to terminate processes exceeding hard limits
- **Tradeoff:** Immediate termination without cleanup
- **Justification:** Ensures strict enforcement and prevents runaway resource usage

---

## Scheduler Experiment Results

To observe scheduling behavior, multiple workloads were executed inside containers under different conditions.

### Experiment Setup

- Two containers (`alpha`, `beta`) were launched simultaneously
- Each container ran CPU-intensive and memory-intensive workloads
- Different memory limits were assigned to observe behavior under constraints

---

### Observations

- Containers running CPU-intensive workloads shared CPU time fairly under default scheduling
- Memory-intensive workloads triggered soft and hard limits as expected
- When limits were exceeded, the kernel module enforced termination reliably
- No starvation was observed between containers under normal conditions

---

### Comparison

| Scenario | Behavior |
|--------|---------|
| Low memory usage | Containers ran normally |
| Soft limit exceeded | Warning generated, execution continued |
| Hard limit exceeded | Process terminated by kernel |
| Multiple containers | Fair scheduling observed |

---

## Important Note

Root filesystem directories (`rootfs-alpha`, `rootfs-base`) are environment-specific and may require setup using tools like `debootstrap`.

---

## Conclusion

This project demonstrates the integration of user-space container runtime design with kernel-level resource monitoring. It highlights how Linux namespaces enable isolation, how kernel modules enable enforcement, and how scheduling and memory management interact in real-world systems.

---

## Authors

- Rakshita Muttur (PES1UG24CS366)
- Rahul M S (PES1UG24CS359)
