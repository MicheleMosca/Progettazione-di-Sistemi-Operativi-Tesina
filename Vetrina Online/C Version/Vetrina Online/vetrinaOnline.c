#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MAX_SCATOLONI 18;

typedef enum {false, true} Boolean;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

int mia_random()
{
    int casuale;    /* variabilie che conterra' il numero casuale */

    casuale = rand() % MAX_SCATOLONI;
    casuale++;  /* incremento il risulatato dato che la rand produce un numero random fra 0 e MAX_SCATOLONI-1, mentre a me serviva un numero fra 1 e MAX_SCATOLONI */

    if ((casuale%2) != 0)   /* mi assicuro che il numero generato sia un multiplo di 2 */
        casuale++;

    return casuale;
}

void *eseguiUtente(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int scatoloni;  /* variabile che contiene il numero di scatoloni che l'utente vuole ordinare */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("UTENTE-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    scatoloni = mia_random();

    printf("%d\n", scatoloni);

    printf("UTENTE-[Thread%d e identificatore %lu] VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiCorriere(void *id)
{
    int *pi = (int *) id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("CORRIERE-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    printf("CORRIERE-[Thread%d e identificatore %lu] VADO A CASA\n", *pi, pthread_self());

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
    int NUM_THREADS_UTENTI;
    int NUM_THREADS_CORRIERI;
    int NUM_THREADS;

    /* Controllo sul numero di parametri */
    if (argc != 3)  /* Soltanto due parametri devono essere passati; Il numero di CORRIERI e il numero di UTENTI */
    {
        sprintf(error, "Errore: Numero dei parametri non corretto. Utilizzo: %s NUMERO_CORRIERI NUMERO_UTENTI \n", argv[0]);
        perror(error);
        exit(1);
    }

    NUM_THREADS_CORRIERI = atoi(argv[1]);
    if (NUM_THREADS_CORRIERI <= 0)   /* Controllo che il numero di thread CORRIERI da creare sia maggiore di zero */
    {
        sprintf(error, "Errore: Numero di thread CORRIERI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(2);
    }

    NUM_THREADS_UTENTI = atoi(argv[2]);
    if (NUM_THREADS_UTENTI <= 0)   /* Controllo che il numero di thread UTENTI da creare sia maggiore di zero */
    {
        sprintf(error, "Errore: Numero di thread UTENTI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(3);
    }

    NUM_THREADS = NUM_THREADS_CORRIERI + NUM_THREADS_UTENTI;

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

    /* Creo i thread CORRIERI */
    for (i = 0; i < NUM_THREADS_CORRIERI; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread CORRIERE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiCorriere, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CORRIERE %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread CORRIERE %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* Creo i thread UTENTE*/
    for (i = NUM_THREADS_CORRIERI; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread UTENTE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiUtente, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread UTENTE %d-esimo\n", taskids[i]);
            perror(error);
            exit(5);
        }
        printf("SONO IL MAIN e ho creato il Pthread UTENTE %i-esimo con id=%lu\n", i, thread[i]);
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

