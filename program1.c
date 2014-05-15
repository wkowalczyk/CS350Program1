#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

int sieveSize;

char* sieveArray;

int count;

void* findPrimes(void* array);

//add a struct with the level and the id of the thread
typedef struct blah{
	int threadNum;
	int id;
} Thread;

int main(int argc, char *argv[])
{

	int shm_fd;
	int *sieve;
	pid_t pid;

	int numWorkers = 0;
	int pFlag = 0;
	int tFlag = 0;

	int j;
	int i;
	int b;
	int a;

	extern char *optarg;
	extern int optind;

	/* Use getopt() to parse the command line arguments and assign values 
	* to the variables. 
	*/
	if(argc == 1){
		exit(0);
	} else{
		while((a = getopt(argc, argv, "uw:s:pt")) != -1){
			switch(a){
				case '?': 
					fprintf(stderr, "./program1 [-u] –w <num_workers> -s <sieve_size> -{pt}\n");
					return 1;
				case 'u':
					fprintf(stderr, "./program1 [-u] –w <num_workers> -s <sieve_size> -{pt}\n");
					return 1;
				case 'w':
					numWorkers = atoi(optarg);
					if(numWorkers > 32){
						fprintf(stderr, "The number of worker processes cannot exceed 32.\n");
						return 1;
					}else{
						break;	
					}
				case 's':
					sieveSize = atoi(optarg);
					if(sieveSize > 100000000){
						fprintf(stderr, "The sieveSize cannot exceed 100,000,000.\n");
						return 1;
					}else{
						break;
					}
				case 'p':
					pFlag = 1;
					break;
				case 't':
					tFlag = 1;
					break;
			}
		}
	}

	startTiming();


	if(pFlag && tFlag){
		//error
		fprintf(stderr, "User is not allowed to set both the pFlag and the tFlag.\nsieve [-u] –w <num_workers> -s <sieve_size> -{pt}\n");
		return 1;
	} else if(tFlag){ //create threads - amount also determined by numWorkers
		pthread_t thread[numWorkers];

		//create the array of size sieveSize numbers
		sieveArray = (char*)malloc((sieveSize)*sizeof(char));
		sieveArray[0] = 1;
		sieveArray[1] = 1;
		for(i = 2; i < sieveSize; i++){
			sieveArray[i] = 0;
		}

		for(i = 0; i < numWorkers; i++){
			//create the threads
			if(pthread_create(&thread[i], NULL, findPrimes, (void*)(i+1)) != 0) {
				perror("pthread_create");
				exit(1);
			}
		}

		for(i = 0; i < numWorkers; i++){
			if(pthread_join(thread[i], NULL) != 0) {
				perror("pthread_join");
				exit(1);
			}
		}

		//for(i = 0; i < sieveSize; i++){
			//printf("array %d: %d\n", i, (int)sieveArray[i]);
		//}

		/*printf("Primes: ");
		for(i = 0; i < sieveSize; i++){
			if(sieveArray[i] == 0){
				printf("%d ", i);		
			}
		}	
		printf("\n");*/

		for(i = 0; i < sieveSize; i++){
			if(sieveArray[i] == 0){
				count++;		
			}
		}	

		printf("Primes less than %d: %d \n\n", sieveSize, count);

		int z = 0;
		for(j = (sieveSize-1); (z<10) && (j > 0); j--){
			if(sieveArray[j] == 0){
				printf("%d\n\n", j);
				z++;
			}		
		}
		
		int* worker = (int*)malloc((numWorkers+1)*sizeof(int));
		
		int count2; 
		for(i = 0; i < numWorkers;i++){
			count2 = 0;
			for(j = 1; j < sieveSize; j++){
				if(sieveArray[j] == i+1){
					count2++;
				}
			}
			printf("Worker %d was the last to knock out %d values\n\n", (i+1), count2);
		}	

	} else if (pFlag) { //create processes.. amount determined by numWorkers
		printf("Process version\n");

		//create the shared memory for the process version
  		/** Change 'name' below to something unique to your program **/
		if ((shm_fd = shm_open("name", O_CREAT | O_RDWR, 0666)) == -1) { 
			perror("shm_open");
			exit(1);
		}
		if (ftruncate(shm_fd, sieveSize * sizeof(int)) == -1) { 
			perror("ftruncate");
			exit(1);
		}

		sieve = mmap(NULL, sieveSize * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

		if (sieve == MAP_FAILED) {
			perror("mmap");
			exit(1);
		} 

	   /* Closing the file descriptor that was used to create the shared memory 
		  region does NOT invalidate the memory. */
		if (close(shm_fd) == -1) {
			perror("close");
			exit(1);
		}

		//initialize the first two locations to 1 
		*sieve = 1;
		*(sieve + 1) = 1;
	
		//initialize all the other locations to 0
		for(j = 2; j < sieveSize; j++){
			*(sieve + j) = 0;
		}

		//fork the child processes
		for(i = 0; i < numWorkers; i++){
			pid = fork();

			if (pid < 0) {  /*error*/ 
				fprintf(stderr, "Fork failed.");
				return 1;
			}

			if (pid == 0) {  /*child process*/ 
				printf("[%d] Child %d.\n", getpid(), i); 
				fflush(stdout);

				//mark the non primes
				for(b = 2; b < sieveSize; b++){
					if(sieve[b] == 0){
						for(j = b+1; j < sieveSize; j++){
							if((j % b) == 0){
								sieve[j] = i+1;
							}
						}
					}
				}
				exit(0);
			}

			else if (i == (numWorkers - 1)){  /*parent process*/
			/* parent will wait for the child to complete */
				for(j = 0; j < numWorkers; j++){
					wait(NULL);
					//printf("Child Complete.\n");	
				}
			}
		}


		for(i = 0; i < sieveSize; i++){
			if(sieve[i] == 0){
				count++;		
			}
		}	

		printf("Primes less than %d: %d \n\n", sieveSize, count);

		int z = 0;
		for(j = (sieveSize-1); (z<10) && (j > 0); j--){
			if(sieve[j] == 0){
				printf("%d\n\n", j);
				z++;
			}		
		}

		
		int* worker = (int*)malloc((numWorkers+1)*sizeof(int));
		
		int count2; 
		for(i = 0; i < numWorkers;i++){
			count2 = 0;
			for(j = 1; j < sieveSize; j++){
				if(sieve[j] == i+1){
					count2++;
				}
			}
			printf("Worker %d was the last to knock out %d values\n\n", (i+1), count2);
		}	


	} else {
		//neither the pFlag or the tFlag were selected
		fprintf(stderr, "User has not specified p or t.\nsieve [-u] –w <num_workers> -s <sieve_size> -{pt}\n");
		return 1;

	}

	stopTiming();

	printf("Time: %d microseconds\n", difference());

	return 1;

}

void * findPrimes(void* array){
	int i;
	int j;	

	for(i = 2; i < sieveSize; i++){
		if(sieveArray[i] == 0){
			for(j = i+1; j < sieveSize; j++){
				if((j % i) == 0){
					sieveArray[j] = (int)array;
				}
			}
		}
	}
	pthread_exit(0);
}
