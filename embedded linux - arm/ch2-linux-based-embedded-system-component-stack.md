
# Linux-based Embedded System Component Statck

Goals:

- To illustrate the main components of Linux-based embedded system
- To analyze the bootstrap process

## Introduction

```
                 +-----------------------------+
               / |         Application         |
Root File System +-----------------------------+
               \ |       System Programs       |
                 +-----------------------------+
               / |    System Call Interface    |
              /  +--------------+--------------+
             +   |   Process    | Virtual File |
             |   |  Management  |    System    |
    Linux Kernel +--------------+--------------+
             |   |    Memory    |    Network   |
             +   |  Management  |  Management  |
              \  +--------------+--------------+
               \ |        Device Drivers       |
                 +--------------+--------------+
                 |  Bootloader  | Device Tree  |
                 +--------------+--------------+
                 |           Hardware          |
                 +-----------------------------+
```

### Bootloader

Software excuted at power-up to set-up the hardware to run operating system.

### Device tree

A tree data structure with nodes that describe the physical devices in the hardware needed by Linux kernel to initialize properly the devices drivers.

### Linux Kernel

The operating system code providing all the services to manager the hardware resources.

### System programs

User-friendly utilities to access operating system services.

### Application

Software implementing the functionalities to be delivered to the embedded system user.

### Root filesystem

Contianer for the Linux kernel configuration files, the system programs, and the application.

### Reference Hardware Model

Embedded System Hardware components:

```
+----------------------------------------+
| Embedded System Hardware               |
|                                        |
|  +--------+  +-----+  +-------------+  |
|  |        |  |     |  | Boot Flash  |  |
|  |        |  | CPU |  +-------------+  |
|  |  RAM   |  |     |                   |
|  | Memory |  +-----+  +-------------+  |
|  |        |  +-----+  | Mass Memory |  |
|  |        |  | I/O |  |    Flash    |  |
|  +--------+  +-----+  +-------------+  |
|                                        |
+----------------------------------------+
```

- RAM memory: volatile memory storing data/code
- CPU: processor running software
- I/O: peripherals to get inputs from the user, and to provide outputs to the user
- Boot Flash: small non-volatile memory needed at power-up
- Mass Memory Flash: large non-volatile memory

Multiple implementions of the reference hardware model are possible.

```
+----------------------------------+
| Microcontroller(MCU)             |
|                                  |
|  +--------+-----+-------------+  |
|  |        |     |             |  |
|  |        | CPU | Boot Flash  |  |
|  |  RAM   |     |             |  |
|  | Memory +-----+-------------+  |
|  |        | I/O | Mass Memory |  |
|  |        |     |    Flash    |  |
|  +--------+-----+-------------+  |
|                                  |
+----------------------------------+
```

- Microcontroller-based implemention: a single device hosts most of the reference model components

```
+------------------------------------------------+
| Microcontroller(MCU)                           |
|                                                |
|              System-on-Chip                    |
|  +--------+  +-------------+  +-------------+  |
|  |        |  |     CPU     |  |             |  |
|  |  RAM   |  +-------+-----+  | Mass Memory |  |
|  | Memory |  | Boot  | I/O |  |    Flash    |  |
|  |        |  | Flash |     |  |             |  |
|  +--------+  +-------+-----+  +-------------+  |
|                                                |
+------------------------------------------------+
```

- System-on-Chip implemention: most of the reference model components are discrete components, while the CPU is integraded with some of them

### CPU Memory Map

The CPU generates 2<sup>N</sup> different addresses (0 -> 2<sup>N</sup>-1).

* N = number of bits of the address bus

Each device (memory, I/O, etc.) is associated with a range of address.

The memory map discribes this association of all the devices.

## Bootloader

### The role of Bootloader

A process is designed to run software from "somewhere".

To start running the software, the process needs to know:

* Where the software is located
* How to get access to the software location
* Where the stack is located 

At power-up, the program counter is set to a default known value, the reset vector.

* The software starting at the reset vector, which will load the bootloader, takes care of providing the process all the needed information.
* The operation performed depend on the system architecture.

### Possible Scenarios

Scenario 1, typical of microcontrollers

* All the software (bootloader + operating system + root filesystem) is stored in persistent storage (boot flash) embedded in the microcontroller.
* All the software is executed from the persistent storage.
* The CPU reset vector is located in the boot flash.
* The RAM memory is embedded in the microcontroller and is used for data, stack and heap only.

Scenario 2, typical of System-on-Chip

* The bootloader is stored into the boot flash.
* The CPU reset vector is located at the boot flash.
* The root filesystem, operating system and device tree are stored in the mass memory flash and loaded in RAM memory by the bootloader.
* The RAM memory is external to the SOC. It will store the operating system + application software, root filesystem (if configured as RAM disk), data, stack, and heap.

### An Example of Bootloader Operations

For a given SoC with the following memory map:

```
+------------+ 0x0003_FFFF
|   RAM      |
|   Memory   |
|            |
|            |
|            |
|            |
+------------+ 0x0001_FFFF
|   Mass     |
|   Memory   |
|   Flash    |
+------------+ 0x0000_FFFF
|            |
| Boot Flash |
|            |
+------------+ 0x0000_0000
```

* RAM memory 512kB (arranged as 128k words, each 32 bits long), from 0x0002_0000 to 0x0003_FFFF
* Mass memory flash 256kB (same origanization as before), from 0x0001_0000 to 0x0001_FFFF
* Boot flash 256kB (same origanization as before), from 0x0000_0000 to 0x0000_FFFF

#### Time: before power up

The boot flash is preloaded with bootloader:

* First bootloader instruction at 0x0000_0FFF
* Last bootloader instruction at 0x0000_AFFF

The mass memory flash is preloaded with:

* Device tree (DT)
* Operating system (OS)
* System programs (SP)
* Application

#### Time: power up

The CPU starts executing software from the reset vector:

* It jumps to the first instruction of the bootloader.
* It runs the bootloader software from boot flash memory.

#### Time: during bootstrap

The CPU executes bootloader software that:

* Initializes the CPU RAM memory controller
* Sets up the CPU registers for mapping stack and heap to RAM memory
* Copies device tree, operating system, system programs, and applications to RAM memory

#### Time: end of bootstrap

The bootloader jumps to the first instruction of the operating system.

The CPU now executes the operating system software, which is responsible for:

* Setting-up the execution environment for the application
* Starting application execution

## Kernel

The Linux Kernel is the software responsible for optimally managing the embedded system's hardware resources.

It offers services such as:

* Process management
* Process scheduling
* Inter-process communication
* Memory management
* I/O management (device drivers)
* File system
* Networking
* And more

The Linux kernel takes advantage of a layered operating system architectrue:

```
              +-----------------------------+
            / |         Application         |
   User Space +-----------------------------+
            \ |       System Programs       |
              +-----------------------------+
            / |    System Call Interface    |
           /  +--------------+--------------+
          +   |   Process    | Virtual File |
          |   |  Management  |    System    |
Kernel Space  +--------------+--------------+
          |   |    Memory    |    Network   |
          |   |  Management  |  Management  |
          +   +--------------+--------------+
           \  |   Device     |   Loadable   |
            \ |   Drivers    |   Module     |
              +--------------+--------------+
```

* The operating system is divided into two layers, one (*user space*) built on top of the other (*Kernel space*).
* User space and Kernel space are different address spaces.
* Basic services are delivered by a single executable, monolithic Kernel.
* Services can be extended at run-time through loadable Kernel modules.

Advantage

- Good separation between application/system programs and kernel. Bugs in the user space do not corrupt the kernel.

Disadvantages

- Bugs in one Kernel component(e.g. a new device driver) can crush the whole system.

## Device tree

To manage hardware resources, the Kernel must know which resources are available in the embedded system (i.e. the hardware discription: I/O devices, memory, etc).

There are two ways to provide this information to the kernel:

* Hardcode it to the Kernel binary code. Each modification to the hardware definition requires recompiling the source code.
* Provide it to the Kernel when the bootloader uses a binary file, the device tree blob.

A device tree blob (DTB) file is produced from a device tree source (DTS)

* A hardware definition can be changed more easily as only DTS recompilation is needed.
* Kernel recompilation is not needed upon changes to the hardware definition. *This is a big time saver*.

## System programs

System programs provide a convenient environment for program development and execution.

They can be divided into:

* Status information
* File modification
* Programming language support
* Program loading and execution
* Communications
* Application programs

## Application

The application is the software required to provide the end user service for which the embedded system was conceived.

Examples can be found in many different products:

* Network Attached storage (NAS)
* Network router
* In-vehicle information
* Specialized lab equipment
* And more

## Root filesystem

The Linux Kernel needs a file system, called a root filesystem, at startup.

* It contians the configuration file needed to prepare the execution environment for the application (e.g. setting up the Ethernet address).
* It contians the first user-level process (init).

The root filesystem can be:

* A portion for the RAM treated as a file system known as Initial RAM disk (initrd), if the embedded system dosen't need to store data persistently during its operations.
* A persistent storage in the embedded system, if the embedded system has to store data persistently during its operations.
* A persistent storage accessed over the network, if developing a Linux-based embedded system.
