# Programming Assignment 4

# Virtual Memory Manager	 
 
This project consists of writing a program that translates logical to physical addresses for a virtual address space of size $2^{16} = 65,536$ bytes. Your program will read from a file containing logical addresses and, using a TLB and a page table, will translate each logical address to its corresponding physical address and output the value of the byte stored at the translated physical address.
Your learning goal is to use simulation to understand the steps involved in translating logical to physical addresses. This will include resolving page faults using demand paging, managing a TLB, and implementing a page-replacement algorithm.

Your program will read a file containing several 32-bit integer numbers that represent logical addresses. However, you need only be concerned with 16-bit addresses, so you must mask the rightmost 16 bits of each logical address. These 16 bits are divided into (1) an 8-bit page number and (2) an 8-bit page offset. Hence, the addresses are structured as shown as:

```text
 31                              16 15           8 7             0
  +--------------------------------+---------------+---------------+
  |                                |     page      |               |
  |                                |    number     |    offset     |
  +--------------------------------+---------------+---------------+
          16 bits                       8 bits          8 bits
```


Other specifics include the following:
1. $2^8$ entries in the page table
2. Page size of $2^8$ bytes
3. $16$ entries in the TLB
4. Frame size of $2^8$ bytes
5. Start with fixed physical memory of $65,536$ bytes (256 frames × 256-byte frame size)

Additionally, your program need only be concerned with reading logical addresses and translating them to their corresponding physical addresses. You do not need to support writing to the logical address space.


## Address Translation
Your program will translate logical to physical addresses using a TLB and page table as outlined in Section 9.3. First, the page number is extracted from the logical address, and the TLB is consulted. In the case of a TLB hit, the frame number is obtained from the TLB. In the case of a TLB miss, the page table must be consulted. In the latter case, either the frame number is obtained from the page table, or a page fault occurs. 


## Handling Page Faults
Your program will implement demand paging as described in Section 10.2. The backing store is represented by the file BACKING STORE.bin, a binary file of size 65,536 bytes. When a page fault occurs, you will read in a 256-byte page from the file BACKING STORE and store it in an available page frame in physical memory. For example, if a logical address with page number 15 resulted in a page fault, your program would read in page 15 from BACKING STORE (remember that pages begin at 0 and are 256 bytes in size) and store it in a page frame in physical memory. Once this frame is stored (and the page table and TLB are updated), subsequent accesses to page 15 will be resolved by either the TLB or the page table.
You will need to treat BACKING STORE.bin as a random-access file so that you can randomly seek to certain positions of the file for reading. We suggest using the standard C library functions for performing I/O, including fopen(), fread(), fseek(), and fclose().
The size of physical memory is the same as the size of the virtual address space so you do not need to be concerned about page replacements during a page fault. Later, we describe a modification to this project using a smaller amount of physical memory; at that point, a page-replacement strategy will be required.

## Test File
We provide the file addresses.txt, which contains integer values representing logical addresses ranging from 0 to 65535 (the size of the virtual address space). Your program will open this file, read each logical address and translate it to its corresponding physical address, and output the value of the signed byte at the physical address.

## How to Begin
First, write a simple program that extracts the page number and offset based
on:

```text
 31                              16 15           8 7             0
  +--------------------------------+---------------+---------------+
  |                                |     page      |               |
  |                                |    number     |    offset     |
  +--------------------------------+---------------+---------------+
          16 bits                       8 bits          8 bits
```

from the following integer numbers: 1, 256, 32768, 32769, 128, 65534, 33153. Perhaps the easiest way to do this is by using the operators for bit-masking and bit-shifting. Once you can correctly establish the page number and offset from an integer number, you are ready to begin.
Initially, we suggest that you bypass the TLB and use only a page table. You can integrate the TLB once your page table is working properly. Remember, address translation can work without a TLB; the TLB just makes it faster. When you are ready to implement the TLB, recall that it has only sixteen entries, so you will need to use a replacement strategy when you update a full TLB. Use FIFO policy for updating your TLB.

## Page Replacement
Thus far, this project has assumed that physical memory is the same size as the virtual address space. In practice, physical memory is typically much smaller than a virtual address space. This phase of the project now assumes using a smaller physical address space with 128 page frames rather than 256. This change will require modifying your program so that it keeps track of free page frames as well as implementing a page-replacement policy using either FIFO or LRU (Section 10.4) to resolve page faults when there is no free memory.

## Modules
You have to translate logical memory to physical and simulate the operations of the main memory structures needed. Below are the units needed to accomplish this. Each of the units below should be modeled as a distinct function, class or data structure in your code.  We only care about read operations for this assignment.
1. **TLB.** The Translation-Lookaside-Buffer is like a small cache for memory translation. An entry in the TLB consist of a logical memory page number and its corresponding physical memory frame number. TLB always gets populated on first-in-first-out (FIFO) basis, meaning the slot populated earliest, is the one that should be replaced by a new entry. Currently empty slots are not given any special priority. We will implement a 16 entry TLB with a rotating FIFO replacement algorithm. 
2. **Page Table.** The page table is a much larger structure that keeps track of every page currently loaded in memory (and pages not currently loaded). It provides the same page to frame translation as TLB. When there is a TLB miss, then we lookup the page number in the page table. If it’s not here either, we incur a page fault. Which means, we have to load the page from the backing store and then update both page table and TLB. 
3. **Backing store or disk device.** This is where the content of all pages can be found. This will be given to you in form of a Unix file called BACKING_STORE.bin of length 65,536 bytes. Every 256 bytes (page size) is a page. Just open it using system calls and read from it as binary. 
4. **Physical memory.** This is where the actual content of memory is loaded. It is also stored in page-size chunks (256 Bytes) called frames. Most often there is not enough physical memory to store all the logically addressable pages. That’s why we have to manage (using a page-replacement algorithm `PRA`) which pages are loaded into which frames. The size of the physical memory (in terms of number of frames `FRAMES`) is passed in via the command-line. 
5. **Reference sequence.** This is a sequence of logical addresses that need to be read. A text file of ASCII integers (one per line) will be given on the command line. Each line is a logical address that needs to  be translated to physical. 

## How to Run Your Program
Your program should run as follows:
`./memsimulator <reference-sequence-file.txt> <FRAMES> <PRA>`

1. `reference-sequence-file` contains the list of logical memory addresses 
2. `FRAMES` is an integer <= 256  and > 0 which is the number of frames in the system. Note that a FRAMES number of 256 means logical and physical memory are of the same size.  
3. `PRA` is `fifo` or `lru` or `opt` standing for a particular page replacement algorithm. 
4. Defaults: `256` for `FRAMES` and `fifo` for `PRA`. 

For example, your program will read in the file `addresses.txt`, which contains 1,000 logical addresses ranging from 0 to 65,535. Your program is to translate each logical address to a physical address and determine the contents of the signed byte stored at the correct physical address. (Recall that in the C language, the char data type occupies a byte of storage, so we suggest using char values.)

Your program is to output the following values:
1. The logical address being translated (the integer value being read from addresses.txt).
2. The corresponding physical address (what your program translates the logical address to).
3. The signed byte value stored in physical memory at the translated physical address.

We also provide the file correct.txt, which contains the correct output values for the file addresses.txt. You should use this file to determine if your program is correctly translating logical to physical addresses.

## Statistics
After completion, your program is to report the following statistics:
1. Page-fault rate —The percentage of address references that resulted in page faults.
2. TLB hit rate — The percentage of address references that were resolved in the TLB.
Since the logical addresses in addresses.txt were generated randomly and do not reflect any memory access locality, do not expect to have a high TLB hit rate.

# Resources
An example is available on the department general use servers:
1. /home/foaad/www/class/453/BACKING_STORE.bin
2. /home/foaad/www/class/453/addresses.txt
3. /home/foaad/www/class/453/addresses-correct.txt 

## Submission
You should submit all of your header, source files along with a makefile.  Your program must compile and run on the department general use servers.  

> **Source & Acknowledgment:** This assignment is reproduced from the “Designing a Virtual Memory Manager” programming project in *Operating System Concepts* by Abraham Silberschatz, Peter B. Galvin, and Greg Gagne. The assignment and supporting materials are publicly available through the book's official website, https://www.os-book.com/. Modifications to this assignment is reproduced from Professor Foaad Khosmood, Cal Poly.