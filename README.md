# cgrunner - Cgroups Process Runner

**cgrunner** is a C++-based command-line application that leverages the Linux Cgroups v2 feature to run processes with specified resource limits. 
The application is designed to control the CPU and memory consumption of processes, helping to manage system resources effectively. 
It creates a dedicated cgroup for each process, sets the desired CPU and memory limits, and ensures that the process runs within these constraints. 
After execution, the cgroup is cleaned up.

## How it Works

1. **Cgroups Managment:**

- The application utilizes **Linux’s Cgroups v2 (Control Groups)** to manage the CPU and memory limits for a given process.
- When the user executes a command, cgrunner creates a new cgroup and applies the specified resource limits (CPU and memory) to that cgroup.
- The `cgrunner process` runs the specified program in the context of the newly created cgroup, ensuring that it is constrained by the defined limits.
- Once the process finishes execution, cgrunner deletes the cgroup to avoid leaving orphaned resources in the system.


2. **Cgroups v2 Integration:**

- The project directly interacts with the Cgroups v2 filesystem to create, configure, and delete cgroups. This is done by manipulating files in the `/sys/fs/cgroup directory`.
- The cgroup is created dynamically for each command execution, and after the command finishes, it is deleted to avoid resource leaks.


3. **Error Handling:**
   
- The application includes basic error handling for common scenarios such as
  - Invalid input values
  - Insufficient permissions
  - Failure to create or delete cgroups
- Provides meaningful error messages and exit codes.


4. **Clean Code and Structure:**

The project follows a clean and modular design, with separate classes and files for different responsibilities:

-	`ArgumentParser`: Handles parsing of command-line arguments.
-	`CgroupManager`: Responsible for creating, managing, and deleting cgroups.
-	`ProcessRunner`: Launches the target process, assigns it to the created cgroup, and ensures proper execution and cleanup.
-	`ErrorUtils`: Provides utility functions for error handling.

## Features

The **cgrunner** application offers the following features:

- **Run processes with CPU and memory limits** – control the resource usage of processes run on the system.
- **Uses Cgroups v2** – leverages the Linux Cgroups v2 mechanism for managing system resources, providing precise control over CPU and memory.
- **Simple CLI usage** – an easy-to-use application that can be run from the terminal, enabling quick management of system resources.
- **Clean and easy-to-understand project structure** – designed with readability and maintainability in mind.

## Requirements

To build and run **cgrunner**, you will need the following:

- **Linux with Cgroups v2 support** – the operating system must support Cgroups v2, which is used for resource management.
- **C++17 or later compiler** – for example, `g++`.
- **CMake version 3.10 or higher** – for project configuration and building.
- **Administrator (root) privileges** – required for manipulating Cgroups and running programs in controlled environments.

## Installation

To build the project, follow these steps:

1. **Clone the repository:**

   To download the project source, use the following command:

   ```bash
   git clone https://github.com/malgorzatapalewicz/cgrunner.git
   cd cgrunner
    ```

2. **Build the project using CMake:**

   To download the project source, use the following command:

   ```bash
   cmake .
   make 
   ```

After building, the cgrunner executable will appear in the working directory.

## Usage

   The application supports one main command `exec`, which allows you to run a process with specified CPU and memory limits.

   ```bash
   sudo ./cgrunner exec --cpu <cpu_percent> --memory <memory_bytes> "<program> [args...]"
   ```
- **`--cpu <cpu_percent>`** – specifies the percentage of CPU the process can use.
- **`--memory <memory_bytes>`** – specifies the memory limit for the process (in bytes).
- **`<program> [args...]`** – the program to run with its arguments.

   
