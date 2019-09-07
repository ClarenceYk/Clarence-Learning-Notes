
# Embedded Linux - Introduction

The Embedded Linux Courese is organized into several modules:

1. Module 1: Linux in Embedded Systems
    - Definition of embedded systems
    - Examples
2. Module 2: Linux-based Embedded System Component Stack
    - Bootloader
    - Kernel
    - Root file system
    - Device tree
    - System programs
    - Application
3. Module 3: Anatomy of a Linux-based System
    - The Liunx kernel internals
    - Device tree
    - System programs and BusyBox
4. Module 4: Configuration & Build Process of an Embedded Linux System
    - Buildroot
    - Yocoto
5. Module 5: Introduction to Linux Kernel Modules
    - CPU - I/O interface
    - I/O taxonomy
    - Linux devices
    - Virtual file system abstraction
    - Linux Kernel modules
6. Module 6: Communication Between Kernel and User Space
    - Module level communication point of view
    - User level communication point of view
7. Module 7: Application Demo: Building a Ranging Sensor Kernel Module
    - The sysfs file system
    - Building Linux support for the HC-SR04 ultrasonic ranging sensor
8. Module 8: System Debugging & Profilling lab exercises

## Introction

Embedded system is a special-purpose computer designed for a specific application. Like a general-purpose computer embedded system has a processor, memory, and input/output(I/O) devices. But it is not the same as conventional, general-purpose computer such as a laptop or desktop.

For example, the processor, memory, and I/O devices that power an embedded system are carefully selected to provide the minimum amount of resources needed to fulfil the requirement of a specific application.

## Embedded System Components

```
                   +-----------------------------+
                   |         Application         |
                   +-----------------------------+
                 / |       System Programs       | \
                /  +-----------------------------+  \
               / / |    System Call Interface    |   \
              / +  +--------------+--------------+    \
             +  |  |   Process    | Virtual File |     \
             |  |  |  Management  |    System    |      +
Basic Software OS  +--------------+--------------+      |
             |  |  |    Memory    |    Network   |   Platform
             +  |  |  Management  |  Management  |      |
              \ +  +--------------+--------------+      +
               \ \ |        Device Drivers       |     /
                \  +-----------------------------+    /
                 \ |          Bootloader         |   /
                   +-----------------------------+  /
                   |           Hardware          | /
                   +-----------------------------+
```

Two main components:
- Application
- Platform

### Application

Software that implements the functionalities for which the embedded system is intended.

### Platform

Combination of hardware and basic software components that provides the services needed for the application to run. Basic software includes system programs, operating system, bootloader.

### Basic Software

- Access to the resources through user-friendly utilities known as *system programs*
- Efficient access to the resources provided by the hardware through the *operating system*
- Initialization of hardware resources at power-up and execution of the operating system through the *bootloader*

### Operating system for embedded systems

- For real-time requirements, real-time system is required
- For supporting for multi-core processing and high-end graphics, Linux or Android, should be considered

## Why Linux-based Emebbed Systems

- Open source
- Engaged community maintaining and improving Linux regulary
- Flexible and adaptable
- Proven in many different scenarios
- Supported by a very large ecosystem of software
- Royalty-free

