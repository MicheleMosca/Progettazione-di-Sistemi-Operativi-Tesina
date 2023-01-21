/* OBIETTIVO: generare un numero non noto di threads OPERAI e AUTO per risolvere il problema dell'OFFICINA.
 * I thread operai sono suddivisi in OPERAI di tipo 0 e OPERAI di tipo 1 mediante il vincolo che ogni tre operai, due devono essere di tipo 0 e uno di tipo 1.
 * Gli operai di tipo 1 sono autorizzati a effettuare controlli per il bollino blu e i tagliandi, mentre gli operai di tipo 0 sono autorizzati soltanto a effettuare i tagliandi.
 * Prima verranno creati i thread OPERAI e successivamente i thread AUTO.
 * I thread OPERAI rimangono in attesa che un auto si presenti per effettuare uno dei due tipi di controlli a disposizione.
 * Una volta che un AUTO entra in officina si inserisce nella coda corrispondente al controllo che deve effettuare, in attesa che un OPERAI la seleziona ed effettua il controllo.
 * I controlli sono simulati mediante sleep di durata random, rispettando il vincolo che i controlli per il bollino blu hanno durata minore rispetto a quelli per il tagliando.
 * Una volta terminato il controllo il thread AUTO ritorna la main il proprio numero d'ordine, mentre il thread OPERAIO si sospenderà in attesa dell'auto successiva */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_DURATA_OPERAZIONE 7
#define AUTO_NON_SERVITA -1
#define AUTO_SERVITA -2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  /* semaforo binario per la mutua esclusione nell'accesso alle procedure entry del monitor */
pthread_cond_t auto_in_coda = PTHREAD_COND_INITIALIZER; /* condition variable in cui gli operai si bloccano in attesa che un auto si metta in coda */
pthread_cond_t attesa_controllo = PTHREAD_COND_INITIALIZER;   /* condition variable in cui le automobili si bloccano in attesa che un operaio prenda in carico la sua richiesta */
pthread_cond_t attesa_termine = PTHREAD_COND_INITIALIZER;   /* condition variable in cui le automobili si bloccano in attesa che un operaio termini il controllo */

int *coda_bollino_blu;  /* array contenente l'id dei threads AUTO in coda per il controllo per il bollino blu */
int contatore_bollino_blu;  /* variabile contatore che indica quante auto sono in coda per il controllo per il bollino blu */

int *coda_tagliando;    /* array contenente l'id dei threads AUTO in coda per il controllo per il tagliando */
int contatore_tagliando;    /* variabile contatore che indica quante auto sono in coda per il controllo per il tagliando */

int *automobili;    /* array di automobili che contiene l'id del thread OPERAIO che la sta servendo, inizializzato a -1 (AUTO_NON_SERITA). Con -2 (AUTO_SERITA) indichiamo che l'operazione di controllo sull'auto e' stata effettuata */

int NUM_THREADS_OPERAI; /* variabile contenente il numero totale di operai scelti per la risoluzione del problema */

int NUM_AUTO_BOLLINO_BLU;   /* variabile che indica il numero di thread AUTO che devono effettuare un controllo per il bollino blu */
int NUM_AUTO_TAGLIANDO; /* variabile che indica il numero di thread AUTO che devono effettuare un controllo per il tagliando */

int mia_random(int MAX)
{
    int casuale;    /* variabile che conterra' il numero casuale */

    casuale = rand() % MAX;
    casuale++;  /* incremento il risultato dato che la rand produce un numero random fra 0 e MAX-1, mentre a me serviva un numero fra 1 e MAX */

    return casuale;
}

void INIZIA_CONTROLLO(int id, int *durata, int tipo, int *id_auto)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    if (tipo == 0)
    {
        /* attendo che ci sia un auto in coda */
        while (contatore_bollino_blu == 0 && contatore_tagliando == 0)  /* e' necessario per il corretto funzionamento di questa soluzione, utilizzare un while per verificare nuovamente la condizione */
        {
            printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] nessun auto e' in coda, mi sospendo\n", tipo, id, pthread_self());
            pthread_cond_wait(&auto_in_coda, &mutex);
        }

        /* verifico il tipo di controllo che devo effettuare, dando precedenza al controllo per il bollino blu */
        if (contatore_bollino_blu > 0)
        {
            printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] auto con tipologia BOLLINO BLU in coda, la rimuovo dalla coda\n", tipo, id, pthread_self());

            /* prendo in carica l'ultima auto che c'e' in coda */
            *id_auto = coda_bollino_blu[contatore_bollino_blu -1];
            automobili[*id_auto - NUM_THREADS_OPERAI] = id;

            /* rimuovo l'auto dalla coda */
            coda_bollino_blu[contatore_bollino_blu -1] = -1;
            contatore_bollino_blu--;

            /* calcolo la durata del controllo */
            *durata = mia_random(MAX_DURATA_OPERAZIONE/2);   /* i controlli per il bollino blu sono piu' veloci rispetto a quello del tagliando */

            printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] prendo in carico l'auto con id %d per un controllo di BOLLINO BLU di durata %d secondi\n", tipo, id, pthread_self(), *id_auto, *durata);
        }
        else
        {
            printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] auto con tipologia TAGLIANDO in coda, la rimuovo dalla coda\n", tipo, id, pthread_self());

            /* prendo in carica l'ultima auto che c'e' in coda */
            *id_auto = coda_tagliando[contatore_tagliando -1];
            automobili[*id_auto - NUM_THREADS_OPERAI] = id;

            /* rimuovo l'auto dalla coda */
            coda_tagliando[contatore_tagliando -1] = -1;
            contatore_tagliando--;

            /* calcolo la durata del controllo */
            *durata = mia_random(MAX_DURATA_OPERAZIONE);   /* i controlli per il bollino blu sono piu' veloci rispetto a quello del tagliando */

            printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] prendo in carico l'auto con id %d per un controllo di TAGLIANDO di durata %d secondi\n", tipo, id, pthread_self(), *id_auto, *durata);
        }
    }
    else
    {
        /* attendo che ci sia un auto in coda */
        while (contatore_tagliando == 0)    /* e' necessario per il corretto funzionamento di questa soluzione, utilizzare un while per verificare nuovamente la condizione */
        {
            printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] nessun auto e' in coda, mi sospendo\n", tipo, id, pthread_self());
            pthread_cond_wait(&auto_in_coda, &mutex);
        }

        printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] auto con tipologia TAGLIANDO in coda, la rimuovo dalla coda\n", tipo, id, pthread_self());

        /* prendo in carica l'ultima auto che c'e' in coda */
        *id_auto = coda_tagliando[contatore_tagliando -1];
        automobili[*id_auto - NUM_THREADS_OPERAI] = id;

        /* rimuovo l'auto dalla coda */
        coda_tagliando[contatore_tagliando -1] = -1;
        contatore_tagliando--;

        /* calcolo la durata del controllo */
        *durata = mia_random(MAX_DURATA_OPERAZIONE);   /* i controlli per il bollino blu sono piu' veloci rispetto a quello del tagliando */
        printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] prendo in carico l'auto con id %d per un controllo di TAGLIANDO di durata %d secondi\n", tipo, id, pthread_self(), *id_auto, *durata);
    }

    /* notifico l'auto di aver appena iniziato il controllo */
    pthread_cond_broadcast(&attesa_controllo);

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void FINE_CONTROLLO(int id, int tipo, int *id_auto)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    printf("OPERAIO_TIPO_%d-[Thread%d e identificatore %lu] controllo sull'auto %d TERMINATO\n", tipo, id, pthread_self(), *id_auto);

    /* modifico lo stato dell'auto in controllo effettuato (-2) */
    automobili[*id_auto - NUM_THREADS_OPERAI] = AUTO_SERVITA;

    /* notifico l'auto del completamento del controllo */
    pthread_cond_broadcast(&attesa_termine);

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void AUTO_ENTRA(int id, int tipo_operazione)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    if (tipo_operazione == 0)
    {
        printf("AUTO-[Thread%d e identificatore %lu] devo effettuare il controllo per il BOLLINO BLU \n", id, pthread_self());

        /* mi metto in coda per i controlli dei bollini blu  */
        coda_bollino_blu[contatore_bollino_blu] = id;
        contatore_bollino_blu++;
    }
    else
    {
        printf("AUTO-[Thread%d e identificatore %lu] devo effettuare il controllo per il TAGLIANDO \n", id, pthread_self());

        /* mi metto in coda per i controlli dei tagliandi  */
        coda_tagliando[contatore_tagliando] = id;
        contatore_tagliando++;
    }

    /* sveglio gli operai in attesa per poter essere servito */
    pthread_cond_broadcast(&auto_in_coda);

    while(automobili[id - NUM_THREADS_OPERAI] == AUTO_NON_SERVITA)  /* e' necessario per il corretto funzionamento di questa soluzione, utilizzare un while per verificare nuovamente la condizione */
    {
        /* mi metto in attesa che un operaio effettui il controllo */
        pthread_cond_wait(&attesa_controllo, &mutex);
    }

    printf("AUTO-[Thread%d e identificatore %lu] l'operario con id %d ha appena preso in carico il mio controllo\n", id, pthread_self(), automobili[id - NUM_THREADS_OPERAI]);

    while (automobili[id - NUM_THREADS_OPERAI] != AUTO_SERVITA) /* e' necessario per il corretto funzionamento di questa soluzione, utilizzare un while per verificare nuovamente la condizione */
    {
        /* il controllo dell'auto e' appena iniziato, attendo il termine */
        pthread_cond_wait(&attesa_termine, &mutex);
    }

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

int generazione_random_tipo_operazione()
{
    int random; /* variabile che conterra' il valore di ritorno della funzione mia_random per la scelta del tipo di operazione che l'auto deve effettuare */

    /* ci sono ancora delle auto bollino blu e delle auto tagliando da generare */
    if (NUM_AUTO_BOLLINO_BLU > 0 && NUM_AUTO_TAGLIANDO > 0)
    {
        /* genero la tipologia dell'operazione che l'auto deve effettuare in modo random */
        random = mia_random(2) - 1; /* sottraggo di 1 perche' la mia_random genera numeri da 1 a MAX, a noi serve da 0 a 1 */

        if (random == 0)    /* operazione selezionata in modo random: Bollino Blu */
        {
            /* diminuisco il numero di auto bollino blu da generare */
            NUM_AUTO_BOLLINO_BLU--;
            return 0;
        }

        /* random ha valore 1, dunque l'operazione da effettuare e' Tagliando, diminuisco il numero di auto ancora da generare */
        NUM_AUTO_TAGLIANDO--;
        return 1;
    }
    else if (NUM_AUTO_BOLLINO_BLU > 0)
    {
        /* sono rimaste solo auto da bollino blu da generare, diminuisco il numero di uno */
        NUM_AUTO_BOLLINO_BLU--;
        return 0;
    }
    else
    {
        /* sono rimaste solo auto tagliando da generare, diminuisco il numero di uno */
        NUM_AUTO_TAGLIANDO--;
        return 1;
    }
}

void *eseguiAuto(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int tipo_operazione;    /* variabile che contiene il tipo di operazione che l'auto deve effettuare, 0 per Bollino Blu e 1 per Tagliando */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("AUTO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    /* effettuo la scelta del tipo di operazione da effettuare */
    tipo_operazione = generazione_random_tipo_operazione();

    AUTO_ENTRA(*pi, tipo_operazione);

    printf("AUTO-[Thread%d e identificatore %lu] controllo terminato, VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiOperaioTipo0(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int durata; /* durata dell'operazione da effettuare */
    int id_auto;    /* variabile che contiene l'id del THREAD AUTO di cui effettueremo il controllo */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("OPERAIO_TIPO_0-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    while(1)
    {
        INIZIA_CONTROLLO(*pi, &durata, 0, &id_auto);

        /* effettuo il controllo */
        sleep(durata);

        FINE_CONTROLLO(*pi, 0, &id_auto);
    }

    /* NB: QUESTA PARTE DI CODICE NON VERRA' MAI ESEGUITA IN QUANTO GLI OPERAI SONO IN UN CICLO INFINITO */

    printf("OPERAIO_TIPO_1-[Thread%d e identificatore %lu] VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiOperaioTipo1(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int durata; /* durata dell'operazione da effettuare */
    int id_auto;    /* variabile che contiene l'id del THREAD AUTO di cui effettueremo il controllo */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    printf("OPERAIO_TIPO_1-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    while(1)
    {
        INIZIA_CONTROLLO(*pi, &durata, 1, &id_auto);

        /* effettuo il controllo */
        sleep(durata);

        FINE_CONTROLLO(*pi, 1, &id_auto);
    }

    /* NB: QUESTA PARTE DI CODICE NON VERRA' MAI ESEGUITA IN QUANTO GLI OPERAI SONO IN UN CICLO INFINITO */

    printf("OPERAIO_TIPO_1-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

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
    int NUM_THREADS_OPERAI_TIPO_0;
    int NUM_THREADS_AUTO;

    /* Controllo sul numero di parametri */
    if (argc != 4)  /* Soltanto tre parametri devono essere passati; Il numero di OPERAI e il numero di AUTO di tipo bollino blue e di tipo tagliando */
    {
        sprintf(error, "Errore: Numero dei parametri non corretto. Utilizzo: %s NUMERO_OPERAI NUMERO_AUTO_BOLLINO_BLU NUMERO_AUTO_TAGLIANDO \n", argv[0]);
        perror(error);
        exit(1);
    }

    NUM_THREADS_OPERAI = atoi(argv[1]);
    if (NUM_THREADS_OPERAI <= 0)   /* Controllo che il numero di thread OPERAI da creare sia maggiore di zero */
    {
        sprintf(error, "Errore: Numero di thread OPERAI insufficienti per l'avvio del programma\n");
        perror(error);
        exit(2);
    }
    printf("Numero totale OPERAI: %d\n", NUM_THREADS_OPERAI);

    NUM_AUTO_BOLLINO_BLU = atoi(argv[2]);
    if (NUM_AUTO_BOLLINO_BLU < 0)   /* Controllo che il numero di thread AUTO_BOLLINO_BLU da creare sia maggiore o uguale a zero */
    {
        sprintf(error, "Errore: Numero di thread AUTO_BOLLINO_BLU insufficienti per l'avvio del programma\n");
        perror(error);
        exit(3);
    }
    printf("Numero AUTO_BOLLINO_BLU: %d\n", NUM_AUTO_BOLLINO_BLU);

    NUM_AUTO_TAGLIANDO = atoi(argv[3]);
    if (NUM_AUTO_TAGLIANDO < 0)   /* Controllo che il numero di thread AUTO_TAGLIANDO da creare sia maggiore o uguale a zero */
    {
        sprintf(error, "Errore: Numero di thread AUTO_TAGLIANDO insufficienti per l'avvio del programma\n");
        perror(error);
        exit(4);
    }
    printf("Numero AUTO_TAGLIANDO: %d\n", NUM_AUTO_TAGLIANDO);

    /* Calcolo il numero di AUTO totale */
    NUM_THREADS_AUTO = NUM_AUTO_BOLLINO_BLU + NUM_AUTO_TAGLIANDO;
    printf("Numero totale di AUTO: %d\n", NUM_THREADS_AUTO);

    /* Controllo che il numero di AUTO non sia uguale a zero */
    if (NUM_THREADS_AUTO == 0)
    {
        sprintf(error, "Errore: Numero di thread AUTO insufficienti per l'avvio del programma\n");
        perror(error);
        exit(5);
    }

    /* Calcolo del numero di THREADS totali */
    NUM_THREADS = NUM_THREADS_OPERAI + NUM_THREADS_AUTO;

    thread = (pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
    if (thread == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di thread\n");
        perror(error);
        exit(6);
    }

    taskids = (int *) malloc(NUM_THREADS * sizeof(int));
    if (taskids == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di taskids\n");
        perror(error);
        exit(7);
    }

    /* Definisco quanti THREADS OPERAI sono di tipo 0 e quanti di tipo 1 mediante il criterio: ogni 3 operai due sono di tipo 0 e uno di tipo 1 */
    if (NUM_THREADS_OPERAI == 1)
        NUM_THREADS_OPERAI_TIPO_0 = 1;
    else if (NUM_THREADS_OPERAI == 2)
        NUM_THREADS_OPERAI_TIPO_0 = 2;
    else
        NUM_THREADS_OPERAI_TIPO_0 = 2 * (NUM_THREADS_OPERAI/3);

    printf("Numero Operai Tipo 0: %d\n", NUM_THREADS_OPERAI_TIPO_0);
    printf("Numero Operai Tipo 1: %d\n", NUM_THREADS_OPERAI - NUM_THREADS_OPERAI_TIPO_0);

    /* Notifico possibilità di Starvation dei thread OPERAI di tipo 1 nel caso non siano presenti AUTO con controllo per il tagliando da effettuare */
    if (NUM_AUTO_TAGLIANDO == 0)
        printf("ATTENZIONE! Si verificherà un caso di Starvation dei %d thread OPERAI_TIPO_1 in quanto il numero di AUTO che vorra' effettuare un controllo per il tagliando e' pari a 0\n", NUM_THREADS_OPERAI - NUM_THREADS_OPERAI_TIPO_0);

    /* Inizializzo l'array di automobili */
    automobili = (int *) malloc(NUM_THREADS_AUTO * sizeof(int));
    if (automobili == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array di automobili\n");
        perror(error);
        exit(8);
    }
    for (i = 0; i < NUM_THREADS_AUTO; i++)
        automobili[i] = AUTO_NON_SERVITA;

    /* Inizializzo l'array della coda di auto per il bollino blu */
    coda_bollino_blu = (int *) malloc(NUM_THREADS_AUTO * sizeof(int));
    if (coda_bollino_blu == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array della coda delle auto per il bollino blu\n");
        perror(error);
        exit(9);
    }
    contatore_bollino_blu = 0;
    for (i = 0; i < NUM_THREADS_AUTO; i++)
    {
        coda_bollino_blu[i] = -1;
    }

    /* Inizializzo l'array della coda di auto per il tagliando */
    coda_tagliando = (int *) malloc(NUM_THREADS_AUTO * sizeof(int));
    if (coda_tagliando == NULL)
    {
        sprintf(error, "Errore: Problemi con l'allocazione dell'array della coda delle auto per il tagliando\n");
        perror(error);
        exit(10);
    }
    contatore_tagliando = 0;
    for (i = 0; i < NUM_THREADS_AUTO; i++)
    {
        coda_tagliando[i] = -1;
    }

    /* Creo i thread OPERAI0_TIPO_0 */
    for (i = 0; i < NUM_THREADS_OPERAI_TIPO_0; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread OPERAIO_TIPO_0 %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiOperaioTipo0, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread OPERAIO_TIPO_0 %d-esimo\n", taskids[i]);
            perror(error);
            exit(11);
        }
        printf("SONO IL MAIN e ho creato il Pthread OPERAIO_TIPO_0 %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* Creo i thread OPERAI0_TIPO_1 */
    for (i = NUM_THREADS_OPERAI_TIPO_0; i < NUM_THREADS_OPERAI; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread OPERAIO_TIPO_1 %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiOperaioTipo1, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread OPERAIO_TIPO_1 %d-esimo\n", taskids[i]);
            perror(error);
            exit(12);
        }
        printf("SONO IL MAIN e ho creato il Pthread OPERAIO_TIPO_1 %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* effettuo una sleep di 1 secondo per accertarmi che gli operai siano tutti operativi prima dell'arrivo delle auto */
    sleep(1);

    /* Creo i thread AUTO */
    for (i = NUM_THREADS_OPERAI; i < NUM_THREADS; i++)
    {
        taskids[i] = i;
        printf("Sto per creare il thread AUTO %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiAuto, (void *) (&taskids[i])) != 0)
        {
            sprintf(error, "Errore: SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread AUTO %d-esimo\n", taskids[i]);
            perror(error);
            exit(13);
        }
        printf("SONO IL MAIN e ho creato il Pthread AUTO %i-esimo con id=%lu\n", i, thread[i]);
    }

    /* Aspetto il termine dei threads AUTO senza aspettare i threads OPERAI_TIPO_0 e OPERAI_TIPO_1 in quanto sono in un ciclo infinito */
    for (i = NUM_THREADS_OPERAI; i < NUM_THREADS; i++)
    {
        int ris;    /* Variabile che memorizza il risultato ritornato dall'esecuzione del thread */
        pthread_join(thread[i], (void**) & p);
        ris = *p;
        printf("Pthread %d-esimo restituisce %d\n", i, ris);
    }

    /* stampa dello stato finale delle code e delle automobili */
    printf("Contatore coda auto bollino blu: %d\n", contatore_bollino_blu);
    printf("Contatore coda auto tagliando: %d\n", contatore_tagliando);
    printf("Stato automobili: [ ");
    for (i = 0; i < NUM_THREADS_AUTO; i++)
    {
        printf("%d ", automobili[i]);
    }
    printf("]\n");

    exit(0);    /* quando il thread main termina, terminano anche gli operai */
}
