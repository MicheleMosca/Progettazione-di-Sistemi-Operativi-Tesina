/* OBIETTIVO: generare un numero non noto di threads CORRIERI e threads UTENTI per la soluzione del problema della VETRINA ONLINE.
 * Prima vengono creati i thread CORRIERI e successivamente i thread UTENTI.
 * I thread CORRIERI rimangono in attesa che un utente effettui un ordine di un numero di scatoloni non noto.
 * Una volta richiesto un ordine da parte del thread UTENTE, il thread CORRIERE che prende in carico l'ordine calcolera' la durata del tragitto e partira' per effettuare tale spedizione.
 * Le spedizioni sono simulate mediante una sleep di durata variabile, scelta del CORRIERE.
 * Una volta consegnato l'ordine il thread CORRIERE rientra in negozio, simulato mediante una sleep della stessa durata del viaggio di andata, per poi dedicarsi alla spedizione successiva.
 * Ogni thread UTENTE una volta ricevuto l'ordine torna al main il proprio numero d'ordine. */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_SCATOLONI 18
#define MAX_DURATA_VIAGGIO 7

#define UTENTE_NON_SERVITO -1
#define UTENTE_SERVITO -2
#define ALTA_PRIORITA 1
#define BASSA_PRIORITA 0

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  /* semaforo binario per la mutua esclusione nell'accesso alle procedure entry del monitor */
pthread_cond_t utenti_in_coda = PTHREAD_COND_INITIALIZER;   /* condition variable in cui i corrieri si bloccano in attesa che un utente si metta in coda */
pthread_cond_t attesa_selezione = PTHREAD_COND_INITIALIZER; /* condition variable in cui gli utenti si bloccano in attesa che un corriere prenda in carico la loro ordinazione */
pthread_cond_t attesa_arrivo = PTHREAD_COND_INITIALIZER;   /* condition variable in cui gli utenti si bloccano in attesa che il corriere gli comunichi che e' arrivato */

int contatore_coda_prioritaria; /* variabile contatore che indica quanti utenti sono in coda per il loro ordine prioritario di 18 scatoloni */
int *coda_prioritaria;  /* array contenente l'id dei threads UTENTI in coda per la presa in carico del loro ordine prioritario */

int contatore_coda_normale; /* variabile contatore che indica quanti utenti sono in coda per il loro ordine non prioritario */
int *coda_normale;  /* array contenente l'id dei threads UTENTI in coda per la presa in carico del loro ordine non prioritario */

int *utenti;    /* array di utenti che contiene l'id del thread CORRIERE che lo sta servendo, inizializzato a -1 (UTENTE_NON_SERVITO). Con -2 (UTENTE_SERVITO) indichiamo che la spedizione e' stata terminata */

int NUM_THREADS_CORRIERI;   /* variabile che indica il numero di threads CORRIERE usati dall'applicazione */

int mia_random(int MAX)
{
    int casuale;    /* variable che conterra' il numero casuale */

    casuale = rand() % MAX;
    casuale++;  /* incremento il risultato dato che la rand produce un numero random fra 0 e MAX-1, mentre a me serviva un numero fra 1 e MAX */

    return casuale;
}

void ORDINA(int id, int scatoloni)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    int priorita;  /* variabile che indica la priorita' di consegna */

    priorita = BASSA_PRIORITA;  /* inizializzo la variabile priorita' a 0 (BASSA_PRIORITA) */

    if (scatoloni == 18)
        priorita = ALTA_PRIORITA;   /* se effettuiamo un ordine con 18 scatoloni allora la priorita' di consegna sara' alta */

    if (priorita == ALTA_PRIORITA)
    {
        printf("UTENTE-[Thread%d e identificatore %lu] voglio ordinare %d scatoloni, mi inserisco nella coda PRIORITARIA \n", id, pthread_self(), scatoloni);

        /* mi inserisco nella coda degli utenti con ALTA priorita' in attesa che un corriere sia libero per effettuare la mia spedizione */
        coda_prioritaria[contatore_coda_prioritaria] = id;
        contatore_coda_prioritaria++;
    }
    else
    {
        printf("UTENTE-[Thread%d e identificatore %lu] voglio ordinare %d scatoloni, mi inserisco nella coda NORMALE \n", id, pthread_self(), scatoloni);

        /* mi inserisco nella coda degli utenti con BASSA priorita' in attesa che un corriere sia libero per effettuare la mia spedizione */
        coda_normale[contatore_coda_normale] = id;
        contatore_coda_normale++;
    }

    /* sveglio i corrieri in attesa, per poter essere servito */
    pthread_cond_broadcast(&utenti_in_coda);

    while(utenti[id - NUM_THREADS_CORRIERI] == UTENTE_NON_SERVITO)  /* verifico la condizione con un while cosi' da accertarmi che la condizione sia ancora soddisfatta dopo il rilascio da parte della wait */
    {
        /* mi sospendo in attesa che un corriere selezioni il mio ordine */
        pthread_cond_wait(&attesa_selezione, &mutex);
    }

    printf("UTENTE-[Thread%d e identificatore %lu] il corriere con id %d ha appena preso in carico il mio ordine\n", id, pthread_self(), utenti[id - NUM_THREADS_CORRIERI]);

    while (utenti[id - NUM_THREADS_CORRIERI] != UTENTE_SERVITO) /* verifico la condizione con un while cosi' da accertarmi che la condizione sia ancora soddisfatta dopo il rilascio da parte della wait */
    {
        /* il corriere e' appena partito, mi sospendo in attesa del suo arrivo */
        pthread_cond_wait(&attesa_arrivo, &mutex);
    }

    printf("UTENTE-[Thread%d e identificatore %lu] il corriere e' appena arrivato, pago e termino l'ordine\n", id, pthread_self());

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void CONSEGNA(int id, int *id_utente, int *durata)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    printf("CORRIERE-[Thread%d e identificatore %lu] ho consegnato l'ordine dell'utente %d, sto rientrando (durata rientro: %d secondi)\n", id, pthread_self(), *id_utente, *durata);

    /* modifico lo stato dell'utente in: UTENTE_SERVITO (-2) */
    utenti[*id_utente - NUM_THREADS_CORRIERI] = UTENTE_SERVITO;

    /* notifico l'utente della consegna dell'ordine */
    pthread_cond_broadcast(&attesa_arrivo);

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void PARTI(int id, int *id_utente, int *durata)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    /* attendo che un utente effettui un ordine */
    while (contatore_coda_prioritaria == 0 && contatore_coda_normale == 0)  /* verifico la condizione con un while cosi' da accertarmi che la condizione sia ancora soddisfatta dopo il rilascio da parte della wait */
    {
        printf("CORRIERE-[Thread%d e identificatore %lu] non ci sono utenti in coda, mi sospendo\n", id, pthread_self());
        pthread_cond_wait(&utenti_in_coda, &mutex); /* mi sospendo in attesa che un utente si metta in una delle code */
    }

    if (contatore_coda_prioritaria > 0) /* controllo se ci sono utenti nella coda prioritaria */
    {
        printf("CORRIERE-[Thread%d e identificatore %lu] un utente e' in coda PRIORITARIA, lo rimuovo dalla coda\n", id, pthread_self());

        /* prendo in carico l'ordine dell'ultimo utente che c'e' in coda */
        *id_utente = coda_prioritaria[contatore_coda_prioritaria - 1];
        utenti[*id_utente - NUM_THREADS_CORRIERI] = id;

        /* rimuovo dalla coda l'utente */
        coda_prioritaria[contatore_coda_prioritaria -1] = -1;
        contatore_coda_prioritaria--;
    }
    else    /* non ci sono utenti nella coda prioritaria, allora gli utenti sono solo nella coda normale */
    {
        printf("CORRIERE-[Thread%d e identificatore %lu] un utente e' in coda NORMALE, lo rimuovo dalla coda\n", id, pthread_self());

        /* prendo in carico l'ordine dell'ultimo utente che c'e' in coda */
        *id_utente = coda_normale[contatore_coda_normale - 1];
        utenti[*id_utente - NUM_THREADS_CORRIERI] = id;

        /* rimuovo dalla coda l'utente */
        coda_normale[contatore_coda_normale -1] = -1;
        contatore_coda_normale--;
    }

    /* calcolo la durata della spedizione */
    *durata = mia_random(MAX_DURATA_VIAGGIO);

    printf("CORRIERE-[Thread%d e identificatore %lu] prendo in carico l'ordine dell'utente con id %d, la spedizione avra' durata di %d secondi\n", id, pthread_self(), *id_utente, *durata);

    /* notifico l'utente di aver preso in carico il suo ordine e che sono appena partito */
    pthread_cond_broadcast(&attesa_selezione);

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
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

    /* genero il numero di scatoloni da ordinare */
    scatoloni = mia_random(MAX_SCATOLONI);

    if ((scatoloni%2) != 0)   /* mi assicuro che il numero generato sia un multiplo di 2 */
        scatoloni++;

    /* chiamo la funzione ORDINA */
    ORDINA(*pi, scatoloni);

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiCorriere(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int durata; /* variabile che contiene la durata del viaggio, per effettuare la spedizione dell'ordine, in secondi */
    int id_utente; /* variabile che contiene l'id dell'utente che il corriere sta servendo */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("CORRIERE-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    while (1)
    {
        /* mi rendo disponibile per la presa in cario di una consegna */
        PARTI(*pi, &id_utente, &durata);

        /* simulo la durata del viaggio mediante una sleep */
        sleep(durata);

        /* effettuo la consegna dell'ordine al cliente */
        CONSEGNA(*pi, &id_utente, &durata);

        /* simulo la durata del viaggio di ritorno in negozio mediante una sleep */
        sleep(durata);
    }

    /* NB: QUESTA PARTE DI CODICE NON VERRA' MAI ESEGUITA IN QUANTO GLI OPERAI SONO IN UN CICLO INFINITO */

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
    int NUM_THREADS;    /* variabile che indica il numero totale di threads */
    int NUM_THREADS_UTENTI; /* variabile che indica il numero di threads UTENTE */

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
        exit(4);
    }

    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di taskids\n");
        perror(error);
        exit(5);
    }

    /* Inizializzo l'array di utenti con valore -1 (UTENTE_NON_SERVITO) */
    utenti = (int *) malloc(NUM_THREADS_UTENTI * sizeof(int));
    if (utenti == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di utenti\n");
        perror(error);
        exit(6);
    }
    for (i = 0; i < NUM_THREADS_UTENTI; i++)
        utenti[i] = UTENTE_NON_SERVITO;

    /* Inizializzo l'array della coda di utenti che hanno effettuato un ordine prioritario, valore iniziale -1 */
    coda_prioritaria = (int *) malloc(NUM_THREADS_UTENTI * sizeof(int));
    contatore_coda_prioritaria = 0;
    if (coda_prioritaria == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array coda prioritaria\n");
        perror(error);
        exit(7);
    }
    for (i = 0; i < NUM_THREADS_UTENTI; i++)
        coda_prioritaria[i] = -1;

    /* Inizializzo l'array della coda di utenti che hanno effettuato un ordine normale, valore iniziale -1 */
    coda_normale = (int *) malloc(NUM_THREADS_UTENTI * sizeof(int));
    contatore_coda_normale = 0;
    if (coda_normale == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array coda normale\n");
        perror(error);
        exit(8);
    }
    for (i = 0; i < NUM_THREADS_UTENTI; i++)
        coda_normale[i] = -1;

    /* Creo i thread CORRIERI */
    for (i = 0; i < NUM_THREADS_CORRIERI; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread CORRIERE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiCorriere, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CORRIERE %d-esimo\n", taskids[i]);
            perror(error);
            exit(6);
        }
        printf("SONO IL MAIN e ho creato il Pthread CORRIERE %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* effettuo una sleep di 1 secondo per assicurarmi che i corrieri sono pronti per ricevere gli ordini degli utenti */
    sleep(1);

    /* Creo i thread UTENTE*/
    for (i = NUM_THREADS_CORRIERI; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread UTENTE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiUtente, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread UTENTE %d-esimo\n", taskids[i]);
            perror(error);
            exit(7);
        }
        printf("SONO IL MAIN e ho creato il Pthread UTENTE %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* Aspetto il termine dei threads UTENTE senza aspettare i threads CORRIERE, in quanto sono in un ciclo infinito */
    for (i = NUM_THREADS_CORRIERI; i < NUM_THREADS; i++)
    {
        int ris;    /* variabile che memorizza il risultato ritornato dall'esecuzione del thread */
        pthread_join(thread[i], (void**) & p);
        ris = *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    /* stampa dello stato finale delle code e degli utenti */
    printf("Contatore coda prioritaria: %d\n", contatore_coda_prioritaria);
    printf("Contatore coda normale: %d\n", contatore_coda_normale);
    printf("Stato utenti: [ ");
    for (i = 0; i < NUM_THREADS_UTENTI; i++)
        printf("%d ", utenti[i]);
    printf("]\n");

    exit(0);    /* quando il thread main termina, terminano anche i corrieri */
}
