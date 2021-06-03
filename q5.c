#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.c"
/* Constants */
#define NB_THREAD 2
/* Global variables */
sem_t queue_access, cons;
struct Queue* queue;
int k = 0;
/* Functions */
int produire(int nb){
    nb++;
    printf("produit: \t%d\n", nb);
    return nb;
}

void consommer(int nb)
{
    const struct timespec delay = {rand() % 3, rand() % 1000000000};
    nanosleep(&delay, NULL);
    printf("consommer: \t%d\n", nb*-1);
}

void deposer(int nb){
    printf("deposer: \t%d\n",nb);
    enqueue(queue, nb);
}

int retirer(){
    int nb = dequeue(queue);
    printf("retirer: \t%d\n",nb);
    return nb;
}

/* Threads functions */
void *production(void *i)
{
    int thread_number = *(int*)i;
    // printf("initialized: Thread production#%d\n", thread_number);

    for (int j = 0; j < 10; j++){
        int nb = produire((thread_number+1)*10+j);
        sem_wait(&queue_access);
        deposer(nb);
        sem_post(&cons);
        sem_post(&queue_access);
    }
}

void *consommation(void *i)
{
    // printf("initialized: Thread consommation#%d\n", *(int*)i);

    int nb;
    for (int j = 0; j < 10; j++){
        sem_wait(&cons);
        sem_wait(&queue_access);
        nb = retirer();
        sem_post(&queue_access);
        consommer(nb);
    }
}

void main () {
    pthread_t prod_threads[NB_THREAD], cons_threads[NB_THREAD];

    queue = createQueue(NB_THREAD);

    /* semaphore sync should be initialized by 0 */
    if (sem_init(&queue_access, 0, 1) == -1) {
        perror("Could not initialize prod semaphore");
        exit(2);
    }
    if (sem_init(&cons, 0, 0) == -1) {
        perror("Could not initialize cons semaphore");
        exit(2);
    }

    /* threads */
    for (int i = 0; i < NB_THREAD; i++)
    {
        if (pthread_create(&prod_threads[i], NULL, production, (void *)&i) < 0) {
            perror("Error: thread cannot be created");
            exit(1);
        }
        if (pthread_create(&cons_threads[i], NULL, consommation, (void *)&i) < 0) {
            perror("Error: thread cannot be created");
            exit(1);
        }
    }
    
    /* wait for created thread to terminate */
    for (int i = 0; i < NB_THREAD; i++)
    {
        pthread_join(prod_threads[i], NULL);
        pthread_join(cons_threads[i], NULL);
    }
    
    /* destroy semaphore sync */
    sem_destroy(&queue_access);
    sem_destroy(&cons);

    free(queue);

    exit(0);
}
