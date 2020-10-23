# Post_Office
Simulation of a post office in pandemic times in C language.

Program uses threads and mutexes,semaphores to synchronize them.

There are waiting 'N' clients in front of the post office.

'N' value is passed to the program as a parameter.

Maximum number of clients that are allowed to stay in the building is 30.

There are 3 windows inside where clients can be served:

  WINDOW_A: time of the service = 300s, max queue length = 5 
  WINDOW_B: time of the service = 150s, max queue length = 10
  WINDOW_C: time of the service = 100s, max queue length = 15
  
Client enters the post office and chooses the best option based on time of the service and number of others in the queue.

In program time passes 1000x faster than in real.

When all clients are served, program prints statistics.
