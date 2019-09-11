
# Anatomy of a Linux-based system

Goals:

- To provide a more detailed view of the Linux architecture
- To illustrate the device tree details
- To introduce the U-BOOT bootloader
- To illustrate the detailed boot process for the UDOO NEO

## Linux architecture

A more in-depth look at the Linux architecture.

### Overview

```
              +-----------------------------+
            / |         Application         |
           |  +-----------------------------+
  User Space  |       System Programs       |
           |  +-----------------------------+
            \ |    GNU C Library (glibc)    |
              +-----------------------------+
            / |    System Call Interface    |
           /  +-----------------------------+
          +   |                             | 
          |   |           Kernel            |
Kernel Space  |                             |
          |   |                             |
          +   +-----------------------------+
           \  |    Board Support Package    |
            \ |           (BSP)             |
              +-----------------------------+
```

Layered architecture based on two levels:

- User space
- Kernel space

User space and kernel space are independent and isolated.

User space and kernel space communicate through special purpose function known as system calls.

#### User Space

Application

- Software implementing the functionalities to be delivered to the embedded system user.

System programs

- User-friendly utilities to access operating sysetm services.

GNU C Library (glibc)

- Interface between the User space and the Kernel space

#### Kernel Space

System call interface

- Entry points to access the services provided by the Kernel (process management, memory management)

Kernel

- Architectrue-independent operating system code
- It implements the hardware-agnostic services of the operating system (e.g. the process scheduler)

Board Support Package (BSP)

- Architecture-dependent operating system code
- It implements the hardware specific services of the operating system (e.g. the context switch)

### Mode details fo the Kernel

```
                                   +--------------------------+
                                   | Memory Management        |
                                   | +----------------------+ |
             +-------------------->+ | Hardware independent | +<---------------+
             |                     | +----------------------+ |                |
             |                     | | Hardware dependent   | |                |
             |                     | +----------------------+ |                |
             |                     +--------------------------+                |
             |                                  ^                              |
             |                                  |                              |
             |                                  |                              |
             v                                  v                              |
+------------+-------------+       +------------+-------------+                |
| Virtual File System      |       | Process Scheduler        |                v
| +----------------------+ |       | +----------------------+ |        +-------+-------+
| | Hardware independent | |       | | Hardware independent | |        | Inter-Process |
| +----------------------+ +<----->+ +----------------------+ +<------>+ communication |
| | Hardware dependent   | |       | | Hardware dependent   | |        +---------------+
| +----------------------+ |       | +----------------------+ |
+--------------------------+       +--------------------------+
             ^                                  ^
             |                                  |
             |                                  |
             |                                  v
             |                     +------------+-------------+
             |                     | Network                  |
             |                     | +----------------------+ |
             |                     | | Hardware independent | |
             +-------------------->+ +----------------------+ |
                                   | | Hardware dependent   | |
                                   | +----------------------+ |
                                   +--------------------------+
```

The Kernel can be divided in five subsystems:

* Process schedular
* Memory manager
* Virtual file system
* Inter-process communication
* Network

Most of them are composed of:

* Hardware-independent code
* Hardware-dependent code

#### Process Scheduler

A process scheduler is responsible for decision making of which processes are running at what time.

Main functions are as follows:

* Processes can create new copies of themselves
* Implements CPU scheduling policy and context switch
* Receives interrupts and routes them to the appropriate kernel subsystem
* Sends signals to user processes
* Manages the hardware timer
* Clean up process resources when a process finishes exection
* Provides support for loadable kernel modules

External interface:

* System calls interface towards the user space (e.g. fork())
* Intra-Kernel interface towards the kernel space (e.g. create_module())

Scheduler tick:

* Directly from system call (e.g. sleep())
* Indirectly after every system call
* After every slow interrupt

> Interrupt type:
>
> - Slow: traditional interrupt (e.g. from a disk driver)
> - Fast: interrupt corresponding to very fast operations (e.g. processing a keyboard input)

#### Memory Management

It is responsible for handing:

* Large address space: user processes can reference more RAM memory than what exists physically
* Protection: the memory for a process is private and cannot be read or modified by another process; also, the memory manager prevents processes from overwriting code and read-only-data
* Memory mapping: processes can map a file into a area of virtual memory and access the file as memory
* Fair access to physical memory: it ensures that processes all have fair access to the memory resources, ensuring reasonable system performance
* Shared memory: it allows processes to share some portion of their memory (e.g. executable code is usually shared amongst processes)

It uses the Memory Management Unit (MMU) to map virtual address to pyhsical address.

- It is conventional for a Liunx system to have a form of MMU support

Advantages:

* Processes can be moved among physical memory maintaining the same virtual addresses
* The same physical memory may be shared among different processes

It swaps process memory out to a paging file when it is not in use:

- Processes using more memory than physically available can be executed.

The *kswapd* Kernel-space process (also known as daemon) is used for this purpose.

* It checks if there are any physical memory pages that haven't been referenced recently
* These pages are evicted from physical memory and stored in a paging file

The MMU detects when a user process accesses a memory address that is not currently mapped to a physical memory location. The MMU notifies the Linux kernel of the event, known as a page fault. The memory manager subsystem will then resolve the page fault.

If the page is currently swapped out to the paging file, it is swapped back in. If the memory manager detects an invalid memory access, it notifies the event to the user process with a signal. If the process doesn’t handle this signal, it is terminated.

##### Memory Manager External Interfaces

System call interface:

* malloc()/free(): allocate or free a region of memory for the process's use
* mmap()/munmap()/msync()/mremap(): map files into virtual memory regions
* mprotect(): change the protection on a region of virtual memory
* mlock()/mlockall()/munlock()/munlockall(): super-user routines to prevent memory being swaped
* swapon()/swapoff(): super-user routines to add and remove swap files for the system

Intra-Kernel interfaces:

* kmalloc()/kfree(): allocate and free memory for use by the kernel's data structures
* verify_area(): verify that a region of user memory is mapped with required permissions
* get_free_page()/free_page(): allocate and free physical memory pages

##### Memory Manager Architecture

```
+-----------------------------------------+
|                                         |
|            +----------------------------|---------+
|            |                            |         |
|     +------+------+            +--------+-------+ |
|     | System call +----------->+  Memory mapped | |
|     | interface   |          +-+  Files         | |
|     +-----+-------+         /  +-------+--------+ |
|           |                /           |          |
|           v               +            v          |
| +---------+-------+       |    +-------+-----+    |
| | Swapfile access |       |    | Core Memory |    |
| |    +------+     +<---------->+ Manager     +<---+
+>+    |kswapd|     |       |    +-----------+-+
  |    +------+     |       |                |
  +---+-------------+       |                |
      |                     |                |
      |                     v                |
      |         +-----------------------+    |
      +-------->+ Architecture-specific +<---+
                | modules               |
                +-----------+-----------+
                            |
                            v
                         +--+--+
                         | MMU |
                         +-----+
```

* System call interface: it provides memory manager services to the user space
* Memory-mapped files: it implements the functionality of memory file mapping algorithms
* Core memory manager: it is responsible for implementing memory allocation algorithms
* Swapfile Access: it controls the paging file access
* Architecture-specific modules: They handle hardware-specific operations related to memory management (e.g. access to the MMU)

## Device trees

## The U-BOOT bootloader

## UDOO NEO boot process