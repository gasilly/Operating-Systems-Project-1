#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define BUFFER_SIZE 15

typedef struct{
	char value;
} item;

struct thread_args{
	FILE *fp;
	int *in;
	int *out;
	int *count;
	int *trips;
	bool doneProducing;
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
	int trips = 0;
	int count = 0;
	FILE *fp = fopen("usenix2019_v3.1.txt", "r");
	if(fp == NULL){
		printf("The file is invalid\n");
		exit(1);
	}
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
	args->count = &count;
	args->trips = &trips;
	args->doneProducing = false;
	args->buffer = buffer;
	args->lock1 = &lock1;
	args->lock2 = &lock2;

	//make producers and consumers
	for(int i = 0; i < 10; i++){
		if((i % 2) != 0){
			pthread_create(&tid[i], NULL, producer, (void *) args);
		}
		else{
			pthread_create(&tid[i],NULL,consumer, (void *) args);
		}
	}
	//close threads/files and free memory  
	for(int i = 0; i < 10; i++){
		pthread_join(tid[i], NULL);
	}
	printf("\nnumber of trips: %d\n", trips);	
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
			args->doneProducing = true;
			break;		
		}
		//lock the file reading so only one thread can access it
		pthread_mutex_lock(args->lock1);
		next_produced.value = fgetc(args->fp);
		if((*(args->in) + 1) % BUFFER_SIZE != *(args->out)){
			*(args->trips) ++;
		}
		while((*(args->count) == BUFFER_SIZE) && ((args->doneProducing) == false));
		args->buffer[*(args->in)] = next_produced;
		*(args->in) = (*(args->in) + 1) % BUFFER_SIZE;
		*(args->count) ++;	
		pthread_mutex_unlock(args->lock1);
	}
	return NULL;
}

//function for consumers to take items from the buffer and print it to the screen
void *consumer(void* _args){
	item next_consumed;
	struct thread_args *args;
	args = (struct thread_args *) _args;

	while(true){
		if(((args->doneProducing) == true) && (*(args->in) == *(args->out))){
			break;
		}
		//lock so only one thread can take from the buffer and print to the screen at a time
		pthread_mutex_lock(args->lock2);	
		while((*(args->count) == 0) && ((args->doneProducing) == false));
		next_consumed = (args->buffer[*(args->out)]);
		printf("%c", next_consumed.value);
		*(args->out) = (*(args->out) + 1) % BUFFER_SIZE;
		*(args->count) --;
		 //consume the next item in next_consumed by printing it to the screen and deleting it from the buffer
		pthread_mutex_unlock(args->lock2);
	}
	return NULL;
}
