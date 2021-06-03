#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
/* Global variables */
int depot;
int k = 0;
sem_t prod;
sem_t cons;
/* Functions */
int produire(int nb){
    nb++;
    printf("produit: \t%d\n", nb);
    return nb;
}

void consommer(int nb)
{
    printf("consommer: \t%d\n", nb*-1);
}

void deposer(int nb){
    printf("deposer: \t%d\n",nb);
    depot = nb;
}

int retirer(){
    int nb = depot;
    depot = 0;
    printf("retirer: \t%d\n",nb);
    return nb;
}

/* Threads functions */
void *production(void *arg)
{
    int i;
    for (i = 0; i < 30; i++){
        int nb = produire(i);
        sem_wait(&prod);
        deposer(nb);
        sem_post(&cons);
    }
}

void *consommation(void *arg)
{
    int i;
    for (i = 0; i < 30; i++){
        sem_wait(&cons);
        int nb = retirer();
        sem_post(&prod);
        consommer(nb);
    }
}

void main () {
    pthread_t prod_thread, cons_thread;
    /* semaphore sync should be initialized by 0 */
    if (sem_init(&prod, 0, 1) == -1) {
        perror("Could not initialize prod semaphore");
        exit(2);
    }
    if (sem_init(&cons, 1, 0) == -1) {
        perror("Could not initialize cons semaphore");
        exit(2);
    }
    if (pthread_create(&prod_thread, NULL, production, NULL) < 0) {
        perror("Error: thread cannot be created");
        exit(1);
    }
    if (pthread_create(&cons_thread, NULL, consommation, NULL) < 0) {
        perror("Error: thread cannot be created");
        exit(1);
    }
    /* wait for created thread to terminate */
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    /* destroy semaphore sync */
    sem_destroy(&prod);
    sem_destroy(&cons);
    exit(0);
}
