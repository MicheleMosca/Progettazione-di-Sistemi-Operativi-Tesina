/* OBIETTIVO:  */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct definizione_gruppo{
    int id;
    int num_persone;
};

typedef struct definizione_gruppo gruppo;

#define PASSEGGERO_NON_SERVITO -1
#define PASSEGGERO_SERVITO -2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  /* semaforo binario per la mutua esclusione nell'accesso alle procedure entry del monitor */

int *coda_passeggeri_singoli;   /* array contenente l'id dei thread PASSEGGERI_SINGOLI in coda per salire sull'elicottero */
int contatore_passeggeri_singoli;   /* variabile contatore che indica quanti passeggeri singoli sono in coda per salire sull'elicottero */

gruppo *coda_passeggeri_gruppo;    /* array di gruppi contenente una struct formata dall'id dei thread PASSEGGERI_GRUPPO in coda per salire sull'elicottero e il relativo numero di persone di cui e' formato il gruppo */
int contatore_passeggeri_gruppo;    /* variabile contatore che indica quanti gruppi di passeggeri sono in coda per salire sull'elicottero */

int *passeggeri;    /* array di passeggeri che contiene l'id del thread PILOTA con cui effettuera' il volo, inizializzato a -1 (PASSEGGERO_NON_SERVITO). Con -2 (PASSEGGERO_SERVITO) indichiamo che il passeggero o gruppo di passeggeri ha terminato il volo */

int NUM_VOLI;   /* variabile che indica il numero di voli che l'elicottero compie durante il giorno */
int NUM_POSTI;  /* variabile che indica il numero di posti che l'elicottero contiene. Deve essere almeno maggiore uguale di due */
int NUM_THREADS_PASSEGGERI_SINGOLI; /* variabile che indica il numero di thread passeggeri singoli che deve essere generato */
int NUM_THREADS_PASSEGGERI_GRUPPO;  /* variabile che indica il numero di thread passeggeri in gruppo che deve essere generato */

int mia_random(int MAX)
{
    int casuale;    /* variabile che conterra' il numero casuale */

    casuale = rand() % MAX;
    casuale++;  /* incremento il risultato dato che la rand produce un numero random fra 0 e MAX-1, mentre a me serviva un numero fra 1 e MAX */

    return casuale;
}

int generazione_random_tipo_di_passeggeri()
{
    int random; /* variabile che conterra' il valore di ritorno della funzione mia_random per la scelta del tipo di passeggeri che si presenta davanti l'elicottero */

    /* ci sono ancora dei passeggeri singoli e passeggeri in gruppo da generare */
    if (NUM_THREADS_PASSEGGERI_SINGOLI > 0 && NUM_THREADS_PASSEGGERI_GRUPPO > 0)
    {
        /* genero la tipologia di passeggeri in modo random */
        random = mia_random(2) - 1; /* sottraggo di 1 perche' la mia_random genera numeri da 1 a MAX, a noi serve da 0 a 1 */

        if (random == 0)    /* tipologia di passeggero selezionata: passeggero singolo */
        {
            /* diminuisco il numero di passeggeri singoli da generare */
            NUM_THREADS_PASSEGGERI_SINGOLI--;
            return 0;
        }

        /* random ha valore 1, dunque il tipo di passeggeri da generare e' in gruppo, diminuisco il numero di passeggeri in gruppo ancora da generare */
        NUM_THREADS_PASSEGGERI_GRUPPO--;
        return 1;
    }
    else if (NUM_THREADS_PASSEGGERI_SINGOLI > 0)
    {
        /* sono rimasti solo passeggeri singoli da generare, diminuisco il numero di uno */
        NUM_THREADS_PASSEGGERI_SINGOLI--;
        return 0;
    }
    else
    {
        /* sono rimasti solo passeggeri in grupo da generare, diminuisco il numero di uno */
        NUM_THREADS_PASSEGGERI_GRUPPO--;
        return 1;
    }
}

void *eseguiPasseggeroSingolo(void *id)
{
    int *pi = (int *) id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("PASSEGGERO_SINGOLO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    /*  */

    printf("PASSEGGERO_SINGOLO-[Thread%d e identificatore %lu] volo terminato, VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiPasseggeriGruppo(void *id)
{
    int *pi = (int *) id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("PASSEGGERI_GRUPPO-[Thread%d e identificatore %lu] STIAMO ARRIVANDO\n", *pi, pthread_self());

    /*  */

    printf("PASSEGGERI_GRUPPO-[Thread%d e identificatore %lu] volo terminato, ANDIAMO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiPilota(void *id)
{
    int *pi = (int *) id;
    int *ptr;

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("PILOTA-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    /*  */

    printf("PILOTA-[Thread%d e identificatore %lu] voli terminati, VADO A CASA\n", *pi, pthread_self());

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
    int NUM_THREADS_PASSEGGERI;

    /* Controllo sul numero di parametri */
    if (argc != 5)  /* Soltanto quattro parametri devono essere passati; Il numero di voli turistici che l'elicottero puo' effettuare, il numero di posti di cui e' composto l'elicottero e il numero di passeggeri singoli e il numero di passeggeri in gruppo */
    {
        sprintf(error, "Errore: Numero dei parametri non corretto. Utilizzo: %s NUMERO_VOLI NUMERO_POSTI NUMERO_PASSEGGERI_SINGOLI NUMERO_PASSEGGERI_GRUPPO \n", argv[0]);
        perror(error);
        exit(1);
    }

    NUM_VOLI = atoi(argv[1]);
    if (NUM_VOLI <= 0)   /* Controllo che il numero di VOLI che l'elicottero deve effettuare sia maggiore di zero */
    {
        sprintf(error, "Errore: Numero di VOLI insufficiente per l'avvio del programma\n");
        perror(error);
        exit(2);
    }
    printf("Numero VOLI da effettuare: %d\n", NUM_VOLI);

    NUM_POSTI = atoi(argv[2]);
    if (NUM_POSTI < 2)   /* Controllo che il numero di POSTI che l'elicottero contiene sia maggiore o uguale a due */
    {
        sprintf(error, "Errore: Numero di POSTI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(3);
    }
    printf("Numero POSTI sull'elicottero: %d\n", NUM_POSTI);

    NUM_THREADS_PASSEGGERI_SINGOLI = atoi(argv[3]);
    if (NUM_THREADS_PASSEGGERI_SINGOLI < 0)   /* Controllo che il numero di thread PASSEGGERI_SINGOLI da creare sia maggiore o uguale a zero */
    {
        sprintf(error, "Errore: Numero di thread PASSEGGERI_SINGOLI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(4);
    }
    printf("Numero PASSEGGERI_SINGOLI: %d\n", NUM_THREADS_PASSEGGERI_SINGOLI);

    NUM_THREADS_PASSEGGERI_GRUPPO = atoi(argv[4]);
    if (NUM_THREADS_PASSEGGERI_GRUPPO < 0)   /* Controllo che il numero di thread PASSEGGERI_GRUPPO da creare sia maggiore o uguale a zero */
    {
        sprintf(error, "Errore: Numero di thread PASSEGGERI_GRUPPO insufficienti per l'avvio del programma\n");
        perror(error);
        exit(5);
    }
    printf("Numero PASSEGGERI_GRUPPO: %d\n", NUM_THREADS_PASSEGGERI_GRUPPO);

    /* Calcolo il numero di PASSEGGERI totale */
    NUM_THREADS_PASSEGGERI = NUM_THREADS_PASSEGGERI_SINGOLI + NUM_THREADS_PASSEGGERI_GRUPPO;
    printf("Numero totale di PASSEGGERI: %d\n", NUM_THREADS_PASSEGGERI);

    /* Controllo che il numero di PASSEGGERI non sia uguale a zero */
    if (NUM_THREADS_PASSEGGERI == 0)
    {
        sprintf(error, "Errore: Numero di thread PASSEGGERI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(6);
    }

    /* Calcolo del numero di THREADS totali */
    NUM_THREADS = NUM_THREADS_PASSEGGERI + 1;   /* il numero di thread passeggeri più il thread pilota */

    thread = (pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di thread\n");
        perror(error);
        exit(7);
    }

    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di taskids\n");
        perror(error);
        exit(8);
    }

    /* Inizializzo l'array di passeggeri */
    passeggeri = (int *) malloc(NUM_THREADS_PASSEGGERI * sizeof(int));
    if (passeggeri == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di passeggeri\n");
        perror(error);
        exit(9);
    }
    for (i = 0; i < NUM_THREADS_PASSEGGERI; i++)
        passeggeri[i] = PASSEGGERO_NON_SERVITO;

    /* Inizializzo l'array della coda dei passeggeri singoli */
    coda_passeggeri_singoli = (int *) malloc(NUM_THREADS_PASSEGGERI_SINGOLI * sizeof(int));
    if (coda_passeggeri_singoli == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array della coda dei passeggeri singoli\n");
        perror(error);
        exit(10);
    }
    contatore_passeggeri_singoli = 0;
    for (i = 0; i < NUM_THREADS_PASSEGGERI_SINGOLI; i++)
    {
        coda_passeggeri_singoli[i] = -1;
    }

    /* Inizializzo l'array della coda di passeggeri in gruppo */
    coda_passeggeri_gruppo = (gruppo *) malloc(NUM_THREADS_PASSEGGERI_GRUPPO * sizeof(gruppo));
    if (coda_passeggeri_gruppo == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array della coda di passeggeri in gruppo\n");
        perror(error);
        exit(11);
    }
    contatore_passeggeri_gruppo = 0;
    for (i = 0; i < NUM_THREADS_PASSEGGERI_GRUPPO; i++)
    {
        coda_passeggeri_gruppo[i].id = -1;
        coda_passeggeri_gruppo[i].num_persone = 0;
    }

    /* Creo il thread PILOTA */
    for (i = 0; i < 1; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread PILOTA %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiPilota, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread PILOTA %d-esimo\n", taskids[i]);
            perror(error);
            exit(12);
        }
        printf("SONO IL MAIN e ho creato il Pthread PILOTA %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* Creo i thread PASSEGGERI */
    for (i = 1; i < NUM_THREADS; i++)
    {
        if (generazione_random_tipo_di_passeggeri() == 0)   /* genero un thread passeggero singolo */
        {
            taskids[i] = i;
            printf("Sto per creare il thread PASSEGGERO_SINGOLO %d-esimo\n", taskids[i]);
            if (pthread_create(&thread[i], NULL, eseguiPasseggeroSingolo, (void *) (&taskids[i])) != 0)
            {
                sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread PASSEGGERO_SINGOLO %d-esimo\n", taskids[i]);
                perror(error);
                exit(13);
            }
            printf("SONO IL MAIN e ho creato il Pthread PASSEGGERO_SINGOLO %i-esimo con id=%lu\n", i, thread[i]);
        }
        else    /* genero un thread passeggeri in gruppo */
        {
            taskids[i] = i;
            printf("Sto per creare il thread PASSEGGERI_GRUPPO %d-esimo\n", taskids[i]);
            if (pthread_create(&thread[i], NULL, eseguiPasseggeriGruppo, (void *) (&taskids[i])) != 0)
            {
                sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread PASSEGGERI_GRUPPO %d-esimo\n", taskids[i]);
                perror(error);
                exit(14);
            }
            printf("SONO IL MAIN e ho creato il Pthread PASSEGGERI_GRUPPO %i-esimo con id=%lu\n", i, thread[i]);
        }
    }

    /* Aspetto il termine di tutti threads generati */
    for (i = 0; i < NUM_THREADS; i++)
    {
        int ris;    /* Variabile che memorizza il risultato ritornato dall'esecuzione del thread */
        pthread_join(thread[i], (void**) & p);
        ris = *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    /* stampa dello stato finale delle code e dei passeggeri */
    printf("Contatore coda passeggeri singoli: %d\n", contatore_passeggeri_singoli);
    printf("Contatore coda passeggeri in gruppo: %d\n", contatore_passeggeri_gruppo);
    printf("Stato passeggeri: [ ");
    for (i = 0; i < NUM_THREADS_PASSEGGERI; i++)
    {
        printf("%d ", passeggeri[i]);
    }
    printf("]\n");

    exit(0);    /* quando il thread main termina, terminano anche gli operai */
}
