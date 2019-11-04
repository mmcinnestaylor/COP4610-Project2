# COP4610-Project2


## Team Members

Keaun Moughari 

Marlan McInnes-Taylor 

Hayden Rogers

## List of Files and Descriptions

Part 1:

* part1.c - makes exactly six system calls
* empty.c - used to compare syscalls
* Makefile - makes source (part1.c and empty.c) to associated part1.x and empty.x executables
* log - first attempt at log to make sure the appropriate amount of calls were made (output of strace on empty.x)
* log2 - verification of appropriate umber of calls (output of strace on part1.x)


Part 2:

* Makefile - makes my_xtime source into the .ko module
* my_xtime.c - procs the time and the time since last proc


Part 3:

* system calls (directory)
  * Makefile - defines obj-y target to add start, issue, and stop to be directly compiled into kernel
  * issue_request.c - issues a request -- places a person on a floor (defines funct pointer, exports symbol, and defines syscall)
  * start_elevator.c - starts the elevator (defines funct pointer, exports symbol, and defines syscall)
  * stop_elevator.c - stops the elevator (defines funct pointer, exports symbol, and defines syscall)
  * syscall_64.tbl - syscall table with existing and new syscalls (adds new syscall names and associated numbers)
  * syscalls.h - sycalls header file for whole kernel (adds new declarations for our added)
  
* elevator (directory)
  * elevator.c - declarations and definitions for elevator kernel module
  * elevator.h - declarations and definitions for all supplementary structures and elevator helper functions
  
* test (directory)
  * Makefile - given (modified for implementing our own stress test)
  * consumer.c - given
  * producer.c - given
  * wrappers.h - given
  * driver.c - added driver to test controlled issue requests (to assure FIFO ordering for loading from waiting queue)


## Makefile description / How to Run

How to run Part 1:
* go to the part1 directory
* call "make .PHONY" to compile and strace both exectuables, outputting to the respective log files
* .PHONY will include a target to sdiff both log files and output to file "diff.txt" 
* NOTE: sdiff returns a negative value so Makefile will print that the target failed, this is not the case, however
* cat diff.txt to see the additional six lines

How to run part 2:
* go to the part2 directory
* call "make .PHONY"; it will compile and insmod automatically
* "cat /proc/timed"
* ???
* Profit?
* call "make remove" to remove the module

How to run part 3:
* go to part3/elevator/test/
* call "make .PHONY1" (this will compile and insert the elevator kernel module)
* call "make start" to start the elevator
* call "make issue" to issue just 1 request
* call "make issues" to issue 50 sequential requests
* call "make stop" to stop the elevator
* "cat /proc/elevator" to print proc entry manually, alternatively the given "watch_proc" will call it every second
* tail -f /var/log/syslog - to see the printks (we have commented out unnecessary prints in addPass, delPass, movePass, and runElevator; if you want them, please uncomment them)


## Documentation of Group Member Contribution

### Meetings
* Group meeting 9/28 - discussed project plan - everyone present
* Group meeting 10/1 - went over part one, made decisions, set up VM - everyone present
* Group meeting 10/4 - tested VM; tested installed kernel - everyone present
* Group meeting 10/9 - part one complete part 2 discussion and planning - everyone present
* Group meeting 10/11 - heavy coding session. pair programming and part 2 development - everyone present
* Group meeting 10/13 - part 2 completed in group meeting. part three planning and discussion - everyone present
* Group meeting 10/19 - part 3 coding and struct/function mapping/planning. codebase discussion and alterations - everyone present
* Group meeting 10/20 - part 3 group coding session - all members present
* Group meeting 10/26 - part 3 logic coding and discussion. module and proc testing - all members present
* Group meeting 10/27 - part 3 implementation of the kernel module. kernel module successfully installed.
* Group meeting 11/31 - part 3 discussion on remaining needs and group programming to get runElevator working.
* Group meeting 11/2 - part 3 group coding marathon. Testing and debugging.
* Group meeting 11/3 - part 3 group coding marathon and successful implementation. Optimization and documentation wrapping up.

### Division of Labor
Due to the nature of this assignment much of the labor was completed in person with all members present for the first and third portions
of the project. In order to learn and gain practice we met frequently and bouced ideas off of each other while trying our own approaches
and meeting back to land on what we considered to be the best take. 

In each group meeting we all mapped out what we wanted to do and discussed how we wanted to divide everything up.
We often met to work off of one machine because it was just easier with everone present given the nature of working on a fragile kernel.
We worked as a group implementing part 1 entirely in person. For part two, we each developed our own version so we could all use that 
as a learning expreince and then we met up and finished it together in a group coding session. For part three, we took a more careful
approach and worked together to carefully plan exactly how we wanted to tackle it and worked to implement the different sections step 
by step making sure to snapshot and not step on each others toes all remoting in to the same machine to work. When working on separate 
functions we mapped them out on paper/whiteboard before implementing them. Debugging and implementation was largely done in-person. If
we were to do it again we would have made more frequent pushes to github, but since we were all working off of the virtual machine 
remotely it was easy to use snapshots and code saved on our personal machines. Also as mentioned above much of the work was completed 
in-person and we never left things in a broken state after splitting up.

All members rate all other members a 10/10 would group again.

### Git commit log

The git commit log is inluded as a .txt file within the tar.

### Assessment of each task/section & known bugs

#### Part 1:

No known issues or bugs.

#### Part 2:

No known issues or bugs.

#### Part 3:

No known issues or bugs.

### EXTRA CREDIT

We are attempting to place in the top five for extra credit.
