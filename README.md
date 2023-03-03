# C-with-linux-project

This is a project in the C language for a course completion, written in a Linux environment.

Project assumptions:

Develop a set of producer-consumer type programs using the mechanism of named links (FIFO queues) to implement the following interprocess communication scheme:

Process 1: Reads data (single rows) from standard input or file and passes them unchanged to process 2.
Process 2: Retrieves data sent by process 1. Calculates the number of characters in each line and sends the result to process 3.
Process 3: Retrieves data produced by process 2 and puts it in standard output. Each received data unit should be outputted in a separate line.

It is necessary to propose and implement a mechanism for processes to inform themselves about their state. Available mechanisms such as signals, semaphores, and shared memory should be utilized for this purpose.

The scenario for notifying processes about their state is as follows: signals are sent to the parent process. The parent process writes the value of the signal to shared memory and sends a notification to process 1 about reading the shared memory. After reading the signal, process 1 sends a notification to process 2 about reading the shared memory. Process 2 notifies process 3 about the need to read the shared memory. All three processes should be automatically called from a single initiating process.

After the system is started, process P1 is called from the menu and we can choose whether the data source is a file or keyboard. If the file input is chosen, the system should prompt for the filename. The data is read and processed by the processes line by line. This continues until the end of the file is reached - then P1 is called from the menu again (it should work continuously). When entering data from the keyboard, lines are distinguished by the enter key and a single dot (".") is used as the stop character.
