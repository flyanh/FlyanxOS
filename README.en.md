# Flyanx

#### Introduction
Flyanx is an open source operating system microkernel, this kernel refers to MINIX and TINIX, the underlying layer uses message mechanism for interprocess communication, the project is highly modular, the code is simple and easy to read. 

#### Software Architecture
Flyanx is based on microkernel architecture design, communication between processes is done using a messaging mechanism. Code hierarchy, Assembly language selects NASM, it is easy to understand and readable. Most of the operating system code is C language, very readable.

### note
The Flyanx kernel temporarily only supports 32-bit I386-based computers; compiles with nasm and gcc compilers;

#### Installation tutorial

1. Clone the source code.
2. In the 32-bit nasm compiler, gcc compiler linux input make all command to compile the system kernel.
3. Enter make image to package the system boot floppy image.
4. Regarding the operation, you can directly put the system floppy disk image on the vbox or other virtual machine to run the viewing effect, or you can enter the make run to run with qemu (provided that your linux installs qemu and installs the i386 architecture).
5. For debugging, you can type make debug to use bochs for debugging (provided you have bochs installed).
6. For more information on installation, please see the Makefile under the project, which contains the complete Chinese annotations and compiled and installed code.

#### Instructions for use

The kernel is not finished yet, so stay tuned...

#### Participate in the contribution

1. Fork warehouse
2. Create a new Feat_xxx branch
3. Commit code
4. New Pull Request
