
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

#### Virtual File System

It is responsible for handling:

* Multiple hardware devices: it provides uniform access to hardware devices.
* Multiple logical file systems: it supports many different logical organizations of information on storage media.
* Multiple executable formats: it supports different executable file formats(e.g. a.out, ELF).
* Homogeneity: it presents a common interface to all of the logical file systems and all hardware devices.
* Performance: it provides high-speed access to files.
* Safety: it enforces policies to not lose or corrupt data.
* Security: it enforces policies to grant access to files only to allowed users, and it restricts user total file size with quotes.

External interfaces:

* System-call interface based on normal operations on file from the POSIX standard (e.g. open/close/write/read).
* Intra-kernel interface based on i-node interface and file interface.

##### i-node

It stores all the information about a file excepts its name and the data it contains.

When a file is created, it is assigned a name and a unique i-node number (a unique integer number).

When a file is accessed

* Each file is associated with a unique i-node number.
* The i-node number is then used for accessing the data structure containing the information about the file being accessed.

i-node interfaces:

* "create", which creates a file in a directory
* "lookup", which finds a file by name within a directory
* "link", "symlink", "unlink", "readlink", or "follow link", which manages file system links
* "mkdir", or "rmdir", which creates or removes a sub-directory
* "mknod",  which creates a directory, special file, or regular file
* "readpage", or "writepage", which reads or writes a page of physical memory
* "truncate", which sets the length of a file to zero
* "permission", which checks to see if a user process has permission to execute an operation
* "smap", which maps a logical file block to a physical device sector
* "bmap", which maps a logical file block to a physical device block
* "rename", which renames a file or directory

File interfaces:

* "open" or "release", which opens or closes the file
* "read" or "write", which reads or writes to the file
* "select", which waits until the file is in a particular state, such as readable or writeable
* "lseek", which moves to a particular offset in the file
* "mmap", which maps a region of the file into the virtual memory of a user process
* "fsync" or "fasync", which synchronizes any memory buffers with the physical device
* "read dir", which reads the files that are pointed to by a directory file
* "i o control", which sets file attributes
* "check media change", which checks to see if a removable media has been removed
* "revalidate", which verifies that all cached information is valid

##### Virtual File System Architecture

```
                                                  +---------------------+
                                                  | Logical file system |
+-----------+                                     |                     |
|System-call+------------------------------------>+   +---+   +---+     |
|interface  |                                     |   |ext|   |nfs|     |
+--+---+----+                       +-------------+   +---+   +---+     |
   |   |                            |             |                     |
   |   |                            |             |   +-----+ +---+     |
   |   |   +-------------------+    |             |   |smbfs| |fat|     |
   |   |   | Binary Executable |    |             |   +-----+ +---+     |
   |   |   |                   |    |             |                     |
   |   |   | +-----+    +---+  +<---+             |   +----+  +----+    |
   |   |   | |a.out|    |ELF|  |                  |   |iofs|  |proc|    |
   |   |   | +-----+    +---+  |                  |   +----+  +----+    |
   |   |   +----------+--------+                  +---------------------+
   |   |              |                                 |
   |   |              |                                 |
   |   |              |                                 v
   |   |              |              +------------------+-----+
   |   |              +------------->+     Buffer cache       |
   |   |                             |                        |
   |   |                             | +------+     +-------+ |
   |   |                             | |buffer|     |kflushd| |
   |   +----------------------+      | +------+     +-------+ |
   |                          |      +------------+-----------+
   |                          |                   |
   |                          |                   |
+--|--------------------------|-------------------|----------------------+
|  |      Device drivers      |                   |                      |
|  v                          v                   v                      |
| ++-------+        +---------+-------------------+--------------------+ |
| |  char  |        |                       block                      | |
| +--------+        +--------------------------------------------------+ |
+------------------------------------------------------------------------+
```

* System call interface: it provides Virtual File System services to user space
* Logical file system: it provides a logical structure for the information stored in a storage medium
  - Several logical file systems are supported (e.g. ext2, fat).
  - All files appear the same to the user.
  - The i-node is used to hide logical file system details.
  - For each file, the corresponding logical file system type is stored in the i-node.
  - Depending on the information in the i-node, the proper operations are activated when reading/writing a file in a given logical file system.
* Buffer cache: it provides data caching mechanisms to improve performance of storage media access operations.
* Binary executable: it supports different types of executable files transparently to the user.
* Device drivers: provide a uniform interface to access hardware devices.
  - Character-based devices are hardware devices access sequentially (e.g. serial port)
  - Block-based devices are devices that are accessed randomly and whose data is read/written in blocks (e.g. hard disk unit).
* Device drivers use the file interface abstraction:
  - Each device can be accessed as a file in the file system through a special file, the device file, associated with it.
  - A new device driver is a new implementing of the hardware-specific code to customize the file interface abstraction.

## Device trees

## The U-BOOT bootloader

## UDOO NEO boot process