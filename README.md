# PA3-Parallel-Image-Processing

## PA Group 98 members

Ryan Bergstein - bergs643

Marwa Osman - osman320

Basma Elaraby - elara006

## CSE computer used

csel-kh1250-03.cselabs.umn.edu

## Changes to Makefile

Added a line to in the "make clean" section to remove the request_log executable.

## Individual Contributions: 

Ryan: Created a group github repository to collectively work out of and set up the project files and README document. Filled out the main function in "image_rotation.c" and helped with the struct logic used in "image_rotation.h".  Did debugging for interim submission. Spent a lot of time reworking and debugging the logic for worker and processor. Also spent time debugging the final submission and fixing errors with output and the mutexs.

Marwa: Wrote the code for the processing function and helped with struct logic. Worked with group members to do final debugging for interim submission. Worked with group members to write code for the worker function and complete the processing function. Helped with debugging.   

Basma: Worked on implementing the thread synchronization using condition variables in "image_rotation.c". Wrote down the plan that regards the final submission. 

## Plan on how you are going to construct the worker threads and how you will make use of mutex locks and unlock and Conditions variables.

1) Construct the Worker Threads
   Initialize mutex locks and condition variables.
   Set up a shared queue to hold image processing tasks.
   Create multiple worker threads in a loop and give each one access to mutex locks, condition variables, a shared task queue, and a flag to signal when task submission is complete.

2) Make Use of Mutex Locks and Condition Variables
   Lock the mutex, wait on a condition variable until the task queue is not empty or the completion flag is set.
   Process the image task– If the angle is 180, perform flip_left_to_right. Otherwise, perform flip_upside_down.
   Save the rotated image using stbi_write_png, update the task queue and processed tasks count.
   Signal the main thread (processing condition) that a task is processed.
   If the completion flag is set and the queue is empty, exit the thread. Otherwise, go back to wait for the next task.


3) Unlock and Signal Condition Variables
   Unlock the mutex after each task is processed.
   Main Thread Work Submission:
   Traverse directory and queue tasks.
   Lock mutex, add tasks, signal workers.
   Set completion flag after all tasks are added.
   Unlock mutex post each addition.
   Wait for task processing to complete.


