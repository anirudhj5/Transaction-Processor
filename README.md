# Transaction-Processor
## Introduction
Multi-thread solution to a transaction processor. Given an input file containing 
accounts and subsequent transactions between these accounts, calculates the ending balance 
after these transactions were made, ans performs any updates based on bank reward rates.

## Background
Multithreading is a way of executing multiple instances of a process 
concurrently, in order to speed up run time and efficiency. The main challenge of using multithreading is 
managing resources across various threads. Incorrect implementations can result in race conditions or deadlock. 
Best practices for multithreading applications involve the use of mutexes, or mutually exclusive flags. By 
wrapping a certain section of code (often known as a critical section) in a mutex, you can essentially lock this 
section of code, only allowing one thread to access the section at a time, thus preventing a race condition. The 
process of thread synchronization involves the use of barriers, which halt the execution of a thread at a certain 
point in the program until all other threads have also reached that point, effectively “synchronizing” the threads. 
However, improper implementation of barriers can cause deadlocks, or indefinite waiting periods where the 
program never continues because one thread never reaches the barrier. 
