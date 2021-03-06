# Post_Office

Console application simulation of a post office in pandemic times in C language.

## Description

* 'N' clients are waiting in front of the post office.

* Maximum number of clients that are allowed to stay in the building is 30.

* Inside the building there are 3 windows where clients can be served

* Each window has different time of the service and max queue length

* Client enters the post office and chooses the best option based on time of the service and number of other clients in the queue.

* Post office is closed when all clients are served


## Additional information

Program uses threads and mutexes,semaphores to synchronize them.

'N' value is passed to the program as a parameter.

There are 3 windows inside the building where clients can be handled:

* window_A: time of the service = 300s, max queue length = 5 
* window_B: time of the service = 150s, max queue length = 10
* window_C: time of the service = 100s, max queue length = 15

Short information is written on the console when client chooses particular queue.

When all clients are served, program prints statistics.

In program time passes 1000x faster than in real.

## Short presentation

![](pres.gif)
