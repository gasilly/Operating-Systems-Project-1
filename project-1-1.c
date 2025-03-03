#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFER_SIZE 10

typedef struct{
	char value;
} item;

struct thread_args{
	FILE *fp;
	int *in;
	int *out;
	item *buffer;
	pthread_mutex_t *lock1;
	pthread_mutex_t *lock2;
};

//Make a structure to pass to pthreads last argument with all the producer function arguments
void* producer(void* _args);
void* consumer(void* _args);

int main(){
	int in = 0;
	int out = 0;
	FILE *fp = fopen("1mb-examplefile-com.txt", "r");
	item buffer[BUFFER_SIZE];
	pthread_t tid[10];
	pthread_mutex_t lock1, lock2;
	pthread_mutex_init(&lock1, NULL);
	pthread_mutex_init(&lock2, NULL);

	//structure setup for multiple arguments in thread call
	struct thread_args *args = malloc(sizeof(struct thread_args));
	args->fp = fp;
	args->in = &in;
	args->out = &out;
	args->buffer = buffer;
	args->lock1 = &lock1;
	args->lock2 = &lock2;

	//make producers
	for(int i = 0; i < 5; i++){
		pthread_create(&tid[i], NULL, producer, (void *) args);
	}
	//make consumers
	for(int i = 5; i < 10; i++){
		pthread_create(&tid[i],NULL,consumer, (void *) args);
	}

	//close threads/files and free memory  
	for(int i = 0; i < 10; i++){
		pthread_join(tid[i], NULL);
	}	
	pthread_mutex_destroy(&lock1);
	pthread_mutex_destroy(&lock2);
	free(args);
	fclose(fp);
	return 0;
}

//function for producers to place items in the buffer
void* producer(void* _args){
	item next_produced;
	struct thread_args *args;
	args = (struct thread_args *) _args;

	while (true) {
		if(feof(args->fp)){
			break;		
		}
		//wait while the buffer is full
		while(((*(args->in) + 1) % BUFFER_SIZE) == *(args->out));
		//lock the file reading so only one thread can access it
		pthread_mutex_lock(args->lock1);
		next_produced.value = fgetc(args->fp);
		args->buffer[*(args->in)] = next_produced;
		*(args->in) = (*(args->in) + 1) % BUFFER_SIZE;
		pthread_mutex_unlock(args->lock1);
	}
}

//function for consumers to take items from the buffer and print it to the screen
void *consumer(void* _args){
	item next_consumed;
	struct thread_args *args;
	args = (struct thread_args *) _args;

	while(true){
		if(feof(args->fp)){
			break;
		}
		//wait while the buffer is empty
		while(*(args->in) == *(args->out));
		//lock so only one thread can take from the buffer and print to the screen at a time
		pthread_mutex_lock(args->lock2);
		next_consumed = (args->buffer[*(args->out)]);
		printf("%c", next_consumed.value);
		*(args->out) = (*(args->out) + 1) % BUFFER_SIZE;
		 //consume the next item in next_consumed by printing it to the screen and deleting it from the buffer
		 pthread_mutex_unlock(args->lock2);
	}
}
