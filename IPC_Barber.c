/*
	compile: gcc -pthread IvaTomevska_hw3.c -o executable
	run: ./executable <NumberOfCustomers> <NumberOfChairs>
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/errno.h>

#define MAX_CUSTOMERS 25

void *customer(void *);
void *barber(void *);
void randwait(int);

/*Define the semaphores*/
sem_t wroom;//limits the # of customers in waiting room
sem_t barberChair;//ensures mutually exclusive acces to barber chair
sem_t barberPillow;//allows barber to sleep until customer arrives
sem_t cuttingHair;//makes customer wait unril barber is done

int allDone = 0;//flag to stop the barber thread when no customers

int main (int argc, char **argv) {
	pthread_t btid;
	pthread_t tid[MAX_CUSTOMERS];
	long RandSeed=2;//for intializing the random number generator, change if needed
	int numCustomers, numChairs;
	int Number[MAX_CUSTOMERS];

	if(argc != 3) {
		printf("Enter the number of customers and number of chairs\n");
		exit(-1);
	}

	/*Get # of customers and chairs in waiting room*/
	numCustomers = atoi(argv[1]);
	numChairs = atoi(argv[2]);
	srand48(RandSeed);//initialize random number generator

	/*Initialize the numbers array*/
	for(int i=0; i<MAX_CUSTOMERS; i++) {
		Number[i]=i;
	}

	/*Initialize the semaphores*/
	sem_init(&wroom, 0, numChairs);
	sem_init(&barberChair, 0, 1);
	sem_init(&barberPillow, 0, 0);
	sem_init(&cuttingHair, 0, 0);

	pthread_create(&btid, NULL, barber, NULL);//Creating barber

	/*Creating customers*/
	for(int i=0; i<numCustomers; i++) {
		pthread_create(&tid[i], NULL, customer, (void *)&Number[i]);
	}

	/*Join threads to wait for them to finnish*/
	for(int i=0; i<numCustomers; i++) {
		pthread_join(tid[i], NULL);
	}

	/*Kill the barber thread when all customers are done*/
	allDone = 1;
	sem_post(&barberPillow);
	pthread_join(btid, NULL);	

}

/*Function for the behaviour of the customers:
Leave if barber shop is full;
If barber is sleepig, wake him up;
If barber is cutting hair, take a chair and wait;
If barber chair is free, give up your space in 
waiting room and take the barber chair;
When barber is done cutting your hair give up the 
barber chair and leave the shop*/
void *customer(void *number){
	int num = *(int *)number;

	/*Leave for the shop and take a random ammount of time to arrive*/
	printf("Customer %d heading toward the barber shop.\n", num);
	randwait(5);
	printf("Customer %d arrived at barber shop.\n", num);

	/*Wait for space to open in the waiting room*/
	if(sem_trywait(&wroom) == -1) {
		printf("Waiting room full. Customer %d is leaving.\n", num);
		return 0;
	}

	sem_wait(&barberChair);//Wait for the barber chair to be free
	sem_post(&wroom);//The chair is free so give up the spot in the waiting room

	/*Wake up the barber*/
	printf("Customer %d waking up the barber.\n", num);
	sem_post(&barberPillow);

	sem_wait(&cuttingHair);//Wait for the barber to cut your hair
	sem_post(&barberChair);//Give up the barber chair.
	printf("Customer %d leaving the shop.\n", num);
}

/*Function for barber behaviour:
Sleep if alone;
When someone wakes you up, sut hair;
release customer when done cutting hair;
If nobody there or on the way, stop the thread*/
void *barber(void *junk) {
	/*If there are customers, don't sleep*/
	while (!allDone) {
		/*Sleep until someone arrives and wakes you*/
		printf("The barber is sleeping.\n");
		sem_wait(&barberPillow);

		/*Work until no more customers*/
		if(!allDone) {
			/*Take a random amont of time to cut hair*/
			printf("The barber is cutting hair.\n");
			randwait(3);
			printf("The barber has finnished cutting hair.\n");

			sem_post(&cuttingHair);//Release the customer when done
		}
		else
			printf("The barber is going home now!\n");
	}
}

/*Random number generator*/
void randwait(int time) {
	int len;
	/*Generate a random number*/
	len = (int)((drand48()*time)+1);
	sleep(len);
}