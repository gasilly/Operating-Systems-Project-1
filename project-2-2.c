#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

//Structure declarations
struct thread_args{
	FILE *fp;
	int search_count;
	pthread_mutex_t *lock;
	pthread_barrier_t *barrier;
};

struct Node{
	char* word;
	struct Node* next;
};

//Globals
int unique_word_count = 0;
struct Node *head = NULL;
struct Node *list_tail = NULL;
char* FILES[] = {"dictionary1.txt", "dictionary2.txt", "dictionary3.txt", "dictionary4.txt"};

//Function Declarations
void* find_unique_words(void* _args);
void print_nodes();
void free_nodes();
int get_words(char *words[], struct thread_args *args);
bool check_unique(struct thread_args* args, struct Node* starting_position, char* word);
struct Node* create_node(char* word);
struct Node* append(char* word);

//Functions used to defined the linked list for storing and adding words, traversing, printing and freeing it
struct Node* create_node(char* word){
	struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
	if(new_node == NULL){
		printf("Error creating a new node.\n");
		exit(0);
	}
	new_node->word = word;
	new_node->next = NULL;
	return new_node;
}

struct Node* append(char* word){
	struct Node* new_node = create_node(word);
	struct Node* temp = head;
	if(head == NULL){
		head = new_node;
		list_tail = new_node;
		return head;
	}
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = new_node;
	list_tail = new_node;
	return head;
}

bool check_unique(struct thread_args* args, struct Node* starting_position, char* word){
	struct Node* temp = starting_position;
	bool unique_word = true;
	while(temp != NULL){
		if(strcmp(temp->word, word) == 0){
			unique_word = false;
			break;
		}
		temp = temp->next;
		args->search_count++;
	}
	return unique_word;
}

void print_nodes(){
	struct Node* temp = head;
	while(temp != NULL){
		printf("%s\n", temp->word);
		temp = temp->next;
	}
}

void free_nodes(){
	struct Node* temp;
	while (head != NULL){
		temp = head;
		head = head->next;
		free(temp);
	}
}

//Using threads and a linked list read four files and copy the unique words from the files into the linked list
int main(){
	pthread_t tid[4];
	pthread_barrier_t barrier;
        pthread_barrier_init(&barrier, NULL, 4);
	pthread_mutex_t lock;
	pthread_mutex_init(&lock, NULL);
	FILE* fp[4];
	struct thread_args args[4];
	//read the files
	for(int i = 0; i < 4; i++){
		fp[i] = fopen(FILES[i], "r");
	        if(fp[i] == NULL){
			printf("Please provide all the required files\n");
			exit(1);
		}
		args[i].fp = fp[i];	
		args[i].lock = &lock;
		args[i].barrier = &barrier;
		args[i].search_count = 0;
	}
	//create the threads
	for(int i = 0; i < 4; i++){
		pthread_create(&tid[i], NULL, &find_unique_words, &args[i]);
	}
	//join the threads once finished
	for(int i = 0; i < 4; i++){
                pthread_join(tid[i], NULL);
        }
	//close the file and destroy the mutex/barrier
	for(int i = 0; i < 4; i++){
		fclose(fp[i]);
	}
	pthread_mutex_destroy(&lock);
	pthread_barrier_destroy(&barrier);
	//print out the final results on the console
	for(int i = 0; i < 4; i++){
		printf("thread %d searched through %d words\n", i + 1, args[i].search_count);
	}
	printf("The unique words found are: \n");
	print_nodes();
	free_nodes();
	return 0;
}

//function to get the unique words from the four files and put them in the linked list
void* find_unique_words(void* _args){
	struct Node *local_tail;
	int num = 0;
	bool is_unique;
	struct thread_args *args;
	args = (struct thread_args *) _args;
	char *words[10];
	for(int i = 0; i < 10; i++){
		words[i] = (char *) malloc(999 * sizeof(char));
	}
	while(num = get_words(words, args)){
		for(int i = 0; i < num; i++){
			is_unique = true;
			local_tail = list_tail;
			is_unique = check_unique(args, head, words[i]);
			if(is_unique == true){
				pthread_mutex_lock(args->lock);
				is_unique = check_unique(args, local_tail, words[i]);
				if(is_unique == true){
					head = append(strdup(words[i]));
				}
				pthread_mutex_unlock(args->lock);
			}
		}
	}
	pthread_barrier_wait(args->barrier);
	for(int i = 0; i < 10; i++){
		free(words[i]);
	}	
}

int get_words(char *words[], struct thread_args *args){
	int word_count = 0;
	for(int i = 0; i < 10; i++){
		if((fgets(words[i], 999, args->fp)) != NULL){
			word_count ++;
			words[i][strcspn(words[i], "\n")] = '\0';
		}
		else{
			break;
		}
	}	
	return word_count;
	
}
 
