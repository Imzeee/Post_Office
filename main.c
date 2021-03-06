#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

#define POST_OFFICE_CAPACITY 30
#define WINDOW_A_CAPACITY 5
#define WINDOW_B_CAPACITY 10
#define WINDOW_C_CAPACITY 15
#define WORKER_A_WORK_TIME 300 // IN MILLISECONDS
#define WORKER_B_WORK_TIME 150
#define WORKER_C_WORK_TIME 100


pthread_mutex_t check_best_queue_mutex;
pthread_mutex_t data_access_protecting_mutex;
sem_t post_office_semaphore;
sem_t worker_A_semaphore;
sem_t worker_B_semaphore;
sem_t worker_C_semaphore;


typedef struct Office_Windows{
	int current_window_A, current_window_B, current_window_C;
}Office_Windows;

typedef struct Statistics_Data{
	int total_window_A, total_window_B, total_window_C;
}Statistics_Data;

Statistics_Data* stats;

long int read_argument(int argc, char* argv[]);

void initialize_mutex_and_semaphores();
void release_mutex_and_semaphores();

void* client_thread(void*);
void* worker_A(void*);
void* worker_B(void*);
void* worker_C(void*);

char check_best_queue(Office_Windows*);
void store_data(char);
void print_statistics(long int number_of_clients);

void open_the_office(Office_Windows*, pthread_t*, long int );

int main(int argc, char* argv[])
{

	long int number_of_clients = read_argument(argc,argv);
	stats = (Statistics_Data*)calloc(3,sizeof(int)); // global structure for statistics
	pthread_t* thread_id = (pthread_t*)malloc(number_of_clients*sizeof(pthread_t));
	if(!thread_id)
	{
		perror("Memory allocation failed!\n");
		exit(EXIT_FAILURE);
	}
	Office_Windows* Actual_Windows_Queue_Lengths = (Office_Windows*)calloc(3,sizeof(int));
	if(!Actual_Windows_Queue_Lengths)
	{
		perror("Memory allocation failed!\n");
		exit(EXIT_FAILURE);
	}

	initialize_mutex_and_semaphores();

	open_the_office(Actual_Windows_Queue_Lengths,thread_id,number_of_clients);

	release_mutex_and_semaphores();

	print_statistics(number_of_clients);

	free(thread_id);
	free(Actual_Windows_Queue_Lengths);
	free(stats);
	return 0;
}


void open_the_office(Office_Windows* Actual_Windows_Queue_Lengths,pthread_t* thread_id, long int number_of_clients)
{
	int result;
	for(int i=0;i<number_of_clients;i++)
	{
		result = pthread_create(&thread_id[i],NULL,client_thread,(void*)Actual_Windows_Queue_Lengths);
		if(result!=0)
		{
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
	}
	for(int i=0;i<number_of_clients;i++)
	{
		result = pthread_join(thread_id[i],NULL);
		if(result != 0)
		{
			perror("Thread join failed");
			exit(EXIT_FAILURE);
		}
	}
}


long int read_argument(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Incorrect number of parameters\nProgram is running with only one parameter!\n");
		exit(EXIT_FAILURE);
	}

	char* endptr = NULL;
	long int number_of_clients = strtol(argv[1], &endptr, 0);

	if(*endptr != '\0')
	{
		printf("Incorrect parameter");
		exit(EXIT_FAILURE);
	}
	if(number_of_clients <= 0)
	{
		printf("Incorrect parameter\nProgram is running with only positive value!\n");
		exit(EXIT_FAILURE);
	}
	if( errno != 0)
	{
		perror("Incorrect parameter \nParameter is out of range (long int)");
		exit(EXIT_FAILURE);
	}

	return number_of_clients;
}

void initialize_mutex_and_semaphores()
{
	int result;
	result = pthread_mutex_init(&check_best_queue_mutex,NULL);
	if(result != 0)
	{
		perror("Mutex creation failed");
		exit(EXIT_FAILURE);
	}

	result = pthread_mutex_init(&data_access_protecting_mutex,NULL);
	if(result != 0)
	{
		perror("Mutex creation failed");
		exit(EXIT_FAILURE);
	}

	result = sem_init(&post_office_semaphore,0,POST_OFFICE_CAPACITY);
	if(result != 0)
	{
		perror("Semaphore creation failed");
		exit(EXIT_FAILURE);
	}
	result = sem_init(&worker_A_semaphore,0,WINDOW_A_CAPACITY);
	if(result != 0)
	{
		perror("Semaphore creation failed");
		exit(EXIT_FAILURE);
	}
	result = sem_init(&worker_B_semaphore,0,WINDOW_B_CAPACITY);
	if(result != 0)
	{
		perror("Semaphore creation failed");
		exit(EXIT_FAILURE);
	}
	result = sem_init(&worker_C_semaphore,0,WINDOW_C_CAPACITY);
	if(result != 0)
	{
		perror("Semaphore creation failed");
		exit(EXIT_FAILURE);
	}
}

void release_mutex_and_semaphores()
{
	sem_destroy(&post_office_semaphore);
	sem_destroy(&worker_A_semaphore);
	sem_destroy(&worker_B_semaphore);
	sem_destroy(&worker_C_semaphore);
	pthread_mutex_destroy(&check_best_queue_mutex);
	pthread_mutex_destroy(&data_access_protecting_mutex);
}

void* client_thread(void* arg) // client behaviour simulation
{
	sem_wait(&post_office_semaphore);

	Office_Windows* Actual_Windows_Queue_Lengths = (Office_Windows*)arg;

	srand(pthread_self()); // using thread ID as a random generator seed
	long random_number = random() % (50 + 1 - 10) + 10; // random number between 10 and 50
	long nanoseconds = random_number * 1000000; // converting miliseconds to nanosecond
	struct timespec observation_time = {.tv_sec=0, .tv_nsec=nanoseconds};
	nanosleep(&observation_time, NULL);


	pthread_mutex_lock(&check_best_queue_mutex);
	char window_letter = check_best_queue(Actual_Windows_Queue_Lengths);
	pthread_mutex_unlock(&check_best_queue_mutex);

	int result;
	pthread_t thread_id;
	switch(window_letter)
	{
		case 'A':
			result = pthread_create(&thread_id,NULL,worker_A,(void*)Actual_Windows_Queue_Lengths);
			if(result!=0)
			{
				perror("Thread creation failed");
				exit(EXIT_FAILURE);
			}
			result = pthread_join(thread_id,NULL);
			if(result!=0)
			{
				perror("Thread joining failed");
				exit(EXIT_FAILURE);
			}
			break;

		case 'B':
			result = pthread_create(&thread_id,NULL,worker_B,(void*)Actual_Windows_Queue_Lengths);
			if(result!=0)
			{
				perror("Thread creation failed");
				exit(EXIT_FAILURE);
			}
			result = pthread_join(thread_id,NULL);
			if(result!=0)
			{
				perror("Thread joining failed");
				exit(EXIT_FAILURE);
			}
			break;

		case 'C':
			result = pthread_create(&thread_id,NULL,worker_C,(void*)Actual_Windows_Queue_Lengths);
			if(result!=0)
			{
				perror("Thread creation failed");
				exit(EXIT_FAILURE);
			}
			result = pthread_join(thread_id,NULL);
			if(result!=0)
			{
				perror("Thread joining failed");
				exit(EXIT_FAILURE);
			}
			break;

		default:
			printf("Queue Chosing Error\n");
			exit(EXIT_FAILURE);

	}

	sem_post(&post_office_semaphore);
	pthread_exit(NULL);
}

void* worker_A(void* arg)
{
	// employee at window_A
	// his work is simulated by sleep function
	// one client can be handled at the time
	Office_Windows* Actual_Windows_Queue_Lengths = (Office_Windows*)arg;
	sem_wait(&worker_A_semaphore);
	Actual_Windows_Queue_Lengths->current_window_A++;
	struct timespec task_doing_time = {.tv_sec=0, .tv_nsec=WORKER_A_WORK_TIME*1000000};
	nanosleep(&task_doing_time,NULL);
	sem_post(&worker_A_semaphore);
	Actual_Windows_Queue_Lengths->current_window_A--;
	pthread_mutex_lock(&data_access_protecting_mutex);
	store_data('A');
	pthread_mutex_unlock(&data_access_protecting_mutex);
	pthread_exit(NULL);
}
void* worker_B(void* arg)
{
	// employee at window_B
	// his work is simulated by sleep function
	// one client can be handled at the time
	Office_Windows* Actual_Windows_Queue_Lengths = (Office_Windows*)arg;
	sem_wait(&worker_B_semaphore);
	Actual_Windows_Queue_Lengths->current_window_B++;
	struct timespec task_doing_time = {.tv_sec=0, .tv_nsec=WORKER_B_WORK_TIME*1000000};
	nanosleep(&task_doing_time,NULL);
	sem_post(&worker_B_semaphore);
	Actual_Windows_Queue_Lengths->current_window_B--;
	pthread_mutex_lock(&data_access_protecting_mutex);
	store_data('B');
	pthread_mutex_unlock(&data_access_protecting_mutex);
	pthread_exit(NULL);
}
void* worker_C(void* arg)
{
	// employee at window_C
	// his work is simulated by sleep function
	// one client can be handled at the time
	Office_Windows* Actual_Windows_Queue_Lengths = (Office_Windows*)arg;
	sem_wait(&worker_C_semaphore);
	Actual_Windows_Queue_Lengths->current_window_C++;
	struct timespec task_doing_time = {.tv_sec=0, .tv_nsec=WORKER_C_WORK_TIME*1000000};
	nanosleep(&task_doing_time,NULL);
	sem_post(&worker_C_semaphore);
	Actual_Windows_Queue_Lengths->current_window_C--;
	pthread_mutex_lock(&data_access_protecting_mutex);
	store_data('C');
	pthread_mutex_unlock(&data_access_protecting_mutex);

	pthread_exit(NULL);
}

char check_best_queue(Office_Windows* Actual_Windows_Queue_Lengths)
{
	// Algorithm to choose best queue depending on actual number of clients in queue and handling time
	int total_waiting_time_A = Actual_Windows_Queue_Lengths->current_window_A * WORKER_A_WORK_TIME;
	int total_waiting_time_B = Actual_Windows_Queue_Lengths->current_window_B * WORKER_B_WORK_TIME;
	int total_waiting_time_C = Actual_Windows_Queue_Lengths->current_window_C * WORKER_C_WORK_TIME;

	if(total_waiting_time_C <= total_waiting_time_B && total_waiting_time_C <= total_waiting_time_A)
	{
		printf("Queue chosen by client: C\n");
		return 'C'; //Worker_C
	}
	else if(total_waiting_time_B <= total_waiting_time_A && total_waiting_time_B <= total_waiting_time_C)
	{
		printf("Queue chosen by client: B\n");
		return 'B'; //Worker_B
	}
	else
	{
		printf("Queue chosen by client: A\n");
		return 'A'; // Worker_A
	}

}

void store_data(char window_letter) // one thread can access data at the time (memory safety)
{
	switch (window_letter)
	{
		case 'A':
			stats->total_window_A++;
			break;
		case 'B':
			stats->total_window_B++;
			break;
		case 'C':
			stats->total_window_C++;
			break;
		default:
			printf("Storing data error\n");
			exit(EXIT_FAILURE);
	}
}

void print_statistics(long int number_of_clients) // printing statistics after closing the post office
{
	printf("\n-----------\n");
	printf("\nStatistics:\n");
	printf("Total number of clients: %ld\n",number_of_clients);
	printf("Window A: %d clients handled\n",stats->total_window_A);
	printf("Window B: %d clients handled\n",stats->total_window_B);
	printf("Window C: %d clients handled\n",stats->total_window_C);
}


