#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define M 7

typedef enum {false, true} Boolean;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

int mia_random()
{
    int casuale;    /* variabilie che conterra' il numero casuale */

    casuale = rand() % 2;
    casuale++;  /* incremento il risulatato dato che la rand produce un numero random fra 0 e 2-1, mentre a me serviva un numero fra 1 e 2 */

    return casuale;
}

void *eseguiCliente(void *id)
{
    int *pi = (int *) id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("CLIENTE-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    printf("CLIENTE-[Thread%d e identificatore %lu] VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

int main(int argc, char **argv)
{
    pthread_t *thread;
    int *taskids;
    int i;
    int *p;
    char error[250];
    int NUM_THREADS;

    /* Controllo sul numero di parametri */
    if (argc != 2)  /* Un solo parametro deve essere ricevuto */
    {
        sprintf(error, "Errore: Numero dei parametri non corretto, sono stati passati %d parametri\n", argc-1);
        perror(error);
        exit(1);
    }

    NUM_THREADS = atoi(argv[1]);
    if (NUM_THREADS <= 0)   /* Controllo che il numero di thread da creare sia maggiore di zero */
    {
        sprintf(error, "Errore: Numero di thread insufficienti per l'avvio del programma\n");
        perror(error);
        exit(2);
    }

    thread = (pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di thread\n");
        perror(error);
        exit(3);
    }

    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di taskids\n");
        perror(error);
        exit(4);
    }

    /* Creo i thread CLIENTE*/
    for (i = 0; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread CLIENTE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiCliente, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CLIENTE %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread CLIENTE %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* Aspetto il termine dei threads */
    for (i = 0; i < NUM_THREADS; i++)
    {
        int ris;    /* Variabile che memorizza il risultato ritornato dall'esecuzione del thread */
        pthread_join(thread[i], (void**) & p);
        ris = *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    exit(0);
}

