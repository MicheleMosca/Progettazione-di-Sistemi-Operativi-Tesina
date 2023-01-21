/* OBIETTIVO: */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_DURATA_APERTURA_CASSETTA 4
#define MAX_DURATA_VISIONE_CASSETTA 8
#define CLIENTE_NON_SERVITO -1
#define CLIENTE_SERVITO -2
#define CLIENTE_NORMALE 0
#define CLIENTE_VIP 1

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  /* semaforo binario per la mutua esclusione nell'accesso alle procedure entry del monitor */
pthread_cond_t clienti_in_coda = PTHREAD_COND_INITIALIZER;  /* condition variable in cui i bancari si bloccano in attesa che un cliente si metta in coda */
pthread_cond_t attesa_ingresso = PTHREAD_COND_INITIALIZER;  /* condition variable in cui i clienti si bloccano in attesa che un bancario lo possa servire */
pthread_cond_t attesa_apertura = PTHREAD_COND_INITIALIZER;  /* condition variable in cui i clienti si bloccano in attesa che il bancario abbia aperto la cassetta di sicurezza */

int *coda_clienti_normali;  /* array contenente l'id dei threads CLIENTE_NORMALE in coda per l'apertura della propria cassetta di sicurezza */
int contatore_clienti_normali;  /* variabile contatore che indica quanti clienti normali sono in coda per l'apertura della propria cassetta di sicurezza */

int *coda_clienti_vip;    /* array contenente l'id dei threads CLIENTE_VIP in coda per l'apertura della propria cassetta di sicurezza */
int contatore_clienti_vip;    /* variabile contatore che indica quanti clienti VIP sono in coda per l'apertura della propria cassetta di sicurezza */

int *clienti_area_risevata; /* array contenente l'id dei threads CLIENTI che attualmente sono all'interno dell'area riservata */
int contatore_clienti_area_riservata;   /* variabile contatore che indica quanti clienti sono attualmente all'interno dell'area riservata */

int *clienti;    /* array di clienti che contiene l'id del thread BANCARIO che lo sta servendo, inizializzato a -1 (CLIENTE_NON_SERVITO). Con -2 (CLIENTE_SERVITO) indichiamo che il bancario ha aperto la cassetta di sicurezza del cliente */

int NUM_THREADS_BANCARI;    /* variabile che indica il numero di threads BANCARI scelto per l'esecuzione del programma */
int MAX_CLIENTI_CONTEMPORANEAMENTE; /* variabile che indica il numero massimo di threads CLIENTI (che comprende sia i CLIENTI_NORMALI che i CLIENTI_VIP) che possono essere presenti contemporaneamente nell'area riservata */
int NUM_THREADS_CLIENTI_NORMALI;    /* variabile che indica il numero di threads CLIENTI_NORMALI scelto per l'esecuzione del programma */
int NUM_THREADS_CLIENTI_VIP;    /* variabile che indica il numero di thread CLIENTI_VIP scelto per l'esecuzione del programma */

int mia_random(int MAX)
{
    int casuale;    /* variabile che conterra' il numero casuale */

    casuale = rand() % MAX;
    casuale++;  /* incremento il risultato dato che la rand produce un numero random fra 0 e MAX-1, mentre a me serviva un numero fra 1 e MAX */

    return casuale;
}

void ENTRA_BANCA(int id, int tipo_cliente)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */



    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void ESCI_BANCA(int id)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */



    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void INIZIO_LAVORO(int id)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */



    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void FINE_LAVORO(int id)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */



    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

int generazione_random_tipo_cliente()
{
    int random; /* variabile che conterra' il valore di ritorno della funzione mia_random per la scelta del tipo di cliente da generare */

    /* ci sono ancora dei clienti VIP e clienti normali da generare */
    if (NUM_THREADS_CLIENTI_VIP > 0 && NUM_THREADS_CLIENTI_NORMALI > 0)
    {
        /* genero la tipologia dell'operazione che l'auto deve effettuare in modo random */
        random = mia_random(2) - 1; /* sottraggo di 1 perche' la mia_random genera numeri da 1 a MAX, a noi serve da 0 a 1 */

        if (random == CLIENTE_NORMALE)    /* cliente genrato in modo random: CLIENTE_NORMALE */
        {
            /* diminuisco il numero di clienti normali da generare */
            NUM_THREADS_CLIENTI_NORMALI--;
            return CLIENTE_NORMALE;
        }

        /* random ha valore 1, dunque il cliente da generare e' un cliente VIP, diminuisco il numero di clienti VIP ancora da generare */
        NUM_THREADS_CLIENTI_VIP--;
        return CLIENTE_VIP;
    }
    else if (NUM_THREADS_CLIENTI_NORMALI > 0)
    {
        /* sono rimasti solo clienti normali da generare, diminuisco il numero di uno */
        NUM_THREADS_CLIENTI_NORMALI--;
        return CLIENTE_NORMALE;
    }
    else
    {
        /* sono rimasti solo clienti VIP da generare, diminuisco il numero di uno */
        NUM_THREADS_CLIENTI_VIP--;
        return CLIENTE_VIP;
    }
}

void *eseguiClienteVIP(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int durata; /* variabile che contiene il numero di secondi, generato in modo random, che il cliente impiega per visionare il contenuto della cassetta di sicurezza */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("CLIENTE_VIP-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    /* entro all'interno della banca */
    ENTRA_BANCA(*pi, CLIENTE_VIP);

    /* genero la durata della visione del contenuto della cassetta di sicurezza */
    durata = mia_random(MAX_DURATA_VISIONE_CASSETTA);

    /* simulo l'azione di visione del contenuto della cassetta di sicurezza mediante una sleep */
    sleep(durata);

    /* notifico di aver appena terminato la visione del contenuto della cassetta di sicurezza */
    ESCI_BANCA(*pi);

    printf("CLIENTE_VIP-[Thread%d e identificatore %lu] contenuto visionato, VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiClienteNormale(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int durata; /* variabile che contiene il numero di secondi, generato in modo random, che il cliente impiega per visionare il contenuto della cassetta di sicurezza */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("CLIENTE_NORMALE-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    /* entro all'interno della banca */
    ENTRA_BANCA(*pi, CLIENTE_NORMALE);

    /* genero la durata della visione del contenuto della cassetta di sicurezza */
    durata = mia_random(MAX_DURATA_VISIONE_CASSETTA);

    /* simulo l'azione di visione del contenuto della cassetta di sicurezza mediante una sleep */
    sleep(durata);

    /* notifico di aver appena terminato la visione del contenuto della cassetta di sicurezza */
    ESCI_BANCA(*pi);

    printf("CLIENTE_NORMALE-[Thread%d e identificatore %lu] contenuto visionato, VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiBancario(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int durata; /* variabile che contiene il numero di secondi, generato in modo random, che il bancario impiega per l'apertura della cassetta di sicurezza  */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("BANCARIO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    while(1)
    {
        /* mi metto a disposizione per l'apertura della cassetta di sicurezza di un utente */
        INIZIO_LAVORO(*pi);

        /* genero in modo random il tempo impiegato per l'apertura della cassetta di sicurezza che mi e' stata assegnata */
        durata = mia_random(MAX_DURATA_APERTURA_CASSETTA);

        /* simulo l'azione di apertura della cassetta di sicurezza mediante una sleep */
        sleep(durata);

        /* notifico all'utente che la sua cassetta di sicurezza e' pronta per essere visionata */
        FINE_LAVORO(*pi);
    }

    /* NB: QUESTA PARTE DI CODICE NON VERRA' MAI ESEGUITA IN QUANTO I BANCARI SONO IN UN CICLO INFINITO */

    printf("BANCARIO-[Thread%d e identificatore %lu] VADO A CASA\n", *pi, pthread_self());

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
    int NUM_THREADS;    /* variabile che contiene il numero totale di threads che verranno creati */
    int NUM_THREADS_CLIENTI;    /* variabile che contiene il numero totale di threads di tipo CLIENTI, dunque la somma tra CLIENTI_NORMALI e CLIENTI_VIP, che verranno creati */

    /* Controllo sul numero di parametri */
    if (argc != 5)  /* Soltanto quattro parametri devono essere passati; Il numero di BANCARI, il numero MAX_CLIENTI_CONTEMPORANEAMENTE (che indica il numero massimo di utenti che possono essere presenti contemporaneamente nell'area riservata), il numero di CLIENTI_NORMALI e il numero di CLIENTI_VIP */
    {
        sprintf(error, "Errore: Numero dei parametri non corretto. Utilizzo: %s BANCARI MAX_CLIENTI_CONTEMPORANEAMENTE CLIENTI_NORMALI CLIENTI_VIP \n", argv[0]);
        perror(error);
        exit(1);
    }

    NUM_THREADS_BANCARI = atoi(argv[1]);
    if (NUM_THREADS_BANCARI <= 0)   /* Controllo che il numero di thread BANCARI da creare sia maggiore di zero */
    {
        sprintf(error, "Errore: Numero di thread BANCARI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(2);
    }
    printf("Numero totale BANCARI: %d\n", NUM_THREADS_BANCARI);

    MAX_CLIENTI_CONTEMPORANEAMENTE = atoi(argv[2]);
    if (MAX_CLIENTI_CONTEMPORANEAMENTE < 2 * NUM_THREADS_BANCARI)   /* Controllo che il numero MAX_CLIENTI_CONTEMPORANEAMENTE sia molto maggiore del numero dei thread BANCARI. Imponendo che per molto maggiore si intende almeno il doppio */
    {
        sprintf(error, "Errore: Numero MAX_CLIENTI_CONTEMPORANEAMENTE insufficienti per l'avvio del programma. Devono essere almeno maggiori del doppio del numero dei threads BANCARI\n");
        perror(error);
        exit(3);
    }
    printf("Numero MAX_CLIENTI_CONTEMPORANEAMENTE: %d\n", MAX_CLIENTI_CONTEMPORANEAMENTE);

    NUM_THREADS_CLIENTI_NORMALI = atoi(argv[3]);
    if (NUM_THREADS_CLIENTI_NORMALI < 0)   /* Controllo che il numero di thread CLIENTI_NORMALI da creare sia maggiore o uguale a zero */
    {
        sprintf(error, "Errore: Numero di thread CLIENTI_NORMALI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(4);
    }
    printf("Numero CLIENTI_NORMALI: %d\n", NUM_THREADS_CLIENTI_NORMALI);

    NUM_THREADS_CLIENTI_VIP = atoi(argv[4]);
    if (NUM_THREADS_CLIENTI_NORMALI < 0)   /* Controllo che il numero di thread CLIENTI_VIP da creare sia maggiore o uguale a zero */
    {
        sprintf(error, "Errore: Numero di thread CLIENTI_VIP insufficienti per l'avvio del programma\n");
        perror(error);
        exit(5);
    }
    printf("Numero CLIENTI_VIP: %d\n", NUM_THREADS_CLIENTI_VIP);

    /* Calcolo il numero di CLIENTI totale */
    NUM_THREADS_CLIENTI = NUM_THREADS_CLIENTI_NORMALI + NUM_THREADS_CLIENTI_VIP;
    printf("Numero totale di CLIENTI: %d\n", NUM_THREADS_CLIENTI);

    /* Controllo che il numero di CLIENTI non sia uguale a zero */
    if (NUM_THREADS_CLIENTI == 0)
    {
        sprintf(error, "Errore: Numero di thread CLIENTI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(6);
    }

    /* Calcolo del numero di THREADS totali */
    NUM_THREADS = NUM_THREADS_BANCARI + NUM_THREADS_CLIENTI;

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

    /* Inizializzo l'array di clienti */
    clienti = (int *) malloc(NUM_THREADS_CLIENTI * sizeof(int));
    if (clienti == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di clienti\n");
        perror(error);
        exit(9);
    }
    for (i = 0; i < NUM_THREADS_CLIENTI; i++)
        clienti[i] = CLIENTE_NON_SERVITO;

    /* Inizializzo l'array della coda di clienti normali */
    coda_clienti_normali = (int *) malloc(NUM_THREADS_CLIENTI_NORMALI * sizeof(int));
    if (coda_clienti_normali == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array della coda dei clienti normali\n");
        perror(error);
        exit(10);
    }
    contatore_clienti_normali = 0;
    for (i = 0; i < NUM_THREADS_CLIENTI_NORMALI; i++)
    {
        coda_clienti_normali[i] = -1;
    }

    /* Inizializzo l'array della coda di clienti VIP */
    coda_clienti_vip = (int *) malloc(NUM_THREADS_CLIENTI_VIP * sizeof(int));
    if (coda_clienti_vip == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array della coda dei clienti VIP\n");
        perror(error);
        exit(11);
    }
    contatore_clienti_vip = 0;
    for (i = 0; i < NUM_THREADS_CLIENTI_VIP; i++)
    {
        coda_clienti_vip[i] = -1;
    }

    /* Inizializzo l'array dei clienti attualmente all'interno dell'area riservata */
    clienti_area_risevata = (int *) malloc(MAX_CLIENTI_CONTEMPORANEAMENTE * sizeof(int));
    if (clienti_area_risevata == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array dei clienti attualmente all'interno dell'area riservata\n");
        perror(error);
        exit(12);
    }
    contatore_clienti_area_riservata = 0;
    for (i = 0; i < MAX_CLIENTI_CONTEMPORANEAMENTE; i++)
    {
        clienti_area_risevata[i] = -1;
    }

    /* Creo i thread BANCARI */
    for (i = 0; i < NUM_THREADS_BANCARI; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread BANCARIO %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiBancario, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread BANCARIO %d-esimo\n", taskids[i]);
            perror(error);
            exit(13);
        }
        printf("SONO IL MAIN e ho creato il Pthread BANCARIO %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* effettuo una sleep di 1 secondo per accertarmi che i bancari siano tutti operativi prima dell'arrivo dei clienti */
    sleep(1);

    /* Creo i thread CLIENTI */
    for (i = NUM_THREADS_BANCARI; i < NUM_THREADS; i++)
    {
        if (generazione_random_tipo_cliente() == CLIENTE_VIP)
        {
            taskids[i] = i;
            printf("Sto per creare il thread CLIENTE_VIP %d-esimo\n", taskids[i]);
            if (pthread_create(&thread[i], NULL, eseguiClienteVIP, (void *) (&taskids[i])) != 0)
            {
                sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CLIENTE_VIP %d-esimo\n", taskids[i]);
                perror(error);
                exit(14);
            }
            printf("SONO IL MAIN e ho creato il Pthread CLIENTE_VIP %i-esimo con id=%lu\n", i, thread[i]);
        }
        else
        {
            taskids[i] = i;
            printf("Sto per creare il thread CLIENTE_NORMALE %d-esimo\n", taskids[i]);
            if (pthread_create(&thread[i], NULL, eseguiClienteNormale, (void *) (&taskids[i])) != 0)
            {
                sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CLIENTE_NORMALE %d-esimo\n", taskids[i]);
                perror(error);
                exit(15);
            }
            printf("SONO IL MAIN e ho creato il Pthread CLIENTE_NORMALE %i-esimo con id=%lu\n", i, thread[i]);
        }
    }

    /* Aspetto il termine dei threads CLIENTI senza aspettare i threads BANCARI in quanto sono in un ciclo infinito */
    for (i = NUM_THREADS_BANCARI; i < NUM_THREADS; i++)
    {
        int ris;    /* Variabile che memorizza il risultato ritornato dall'esecuzione del thread */
        pthread_join(thread[i], (void**) & p);
        ris = *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    /* stampa dello stato finale delle code e dei clienti */
    printf("Contatore coda clienti VIP: %d\n", contatore_clienti_vip);
    printf("Contatore coda clienti normali: %d\n", contatore_clienti_normali);
    printf("Stato clienti: [ ");
    for (i = 0; i < NUM_THREADS_CLIENTI; i++)
    {
        printf("%d ", clienti[i]);
    }
    printf("]\n");

    exit(0);    /* quando il thread main termina, terminano anche i bancari */
}
