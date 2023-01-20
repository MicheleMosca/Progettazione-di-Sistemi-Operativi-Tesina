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
typedef enum {false, true} Boolean;

#define PASSEGGERO_NON_SERVITO -1
#define PASSEGGERO_SERVITO -2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  /* semaforo binario per la mutua esclusione nell'accesso alle procedure entry del monitor */
pthread_cond_t attesa_imbarco = PTHREAD_COND_INITIALIZER; /* condition variable in cui i passeggeri singoli/in gruppo si bloccano in attesa che il pilota li faccia salire sull'elicottero */
pthread_cond_t attesa_termine = PTHREAD_COND_INITIALIZER; /* condition variable in cui i passeggeri singoli/in gruppo si bloccano in attesa che il volo termini */

int *coda_passeggeri_singoli;   /* array contenente l'id dei thread PASSEGGERI_SINGOLI in coda per salire sull'elicottero */
int contatore_passeggeri_singoli;   /* variabile contatore che indica quanti passeggeri singoli sono in coda per salire sull'elicottero */

gruppo *coda_passeggeri_gruppo;    /* array di gruppi contenente una struct formata dall'id dei thread PASSEGGERI_GRUPPO in coda per salire sull'elicottero e il relativo numero di persone di cui e' formato il gruppo */
int contatore_passeggeri_gruppo;    /* variabile contatore che indica quanti gruppi di passeggeri sono in coda per salire sull'elicottero */

int *passeggeri;    /* array di passeggeri che contiene l'id del thread PILOTA con cui effettuera' il volo, inizializzato a -1 (PASSEGGERO_NON_SERVITO). Con -2 (PASSEGGERO_SERVITO) indichiamo che il passeggero o gruppo di passeggeri ha terminato il volo */

int NUM_VOLI;   /* variabile che indica il numero di voli che l'elicottero compie durante il giorno */
int NUM_POSTI;  /* variabile che indica il numero di posti che l'elicottero contiene. Deve essere almeno maggiore uguale di due */
int NUM_THREADS_PASSEGGERI_SINGOLI; /* variabile che indica il numero di thread passeggeri singoli che deve essere generato */
int NUM_THREADS_PASSEGGERI_GRUPPO;  /* variabile che indica il numero di thread passeggeri in gruppo che deve essere generato */

int voli_eseguiti;  /* variabile contatore che serve a memorizzare il numero di voli che il pilota ha effettuato */

int mia_random(int MAX)
{
    int casuale;    /* variabile che conterra' il numero casuale */

    casuale = rand() % MAX;
    casuale++;  /* incremento il risultato dato che la rand produce un numero random fra 0 e MAX-1, mentre a me serviva un numero fra 1 e MAX */

    return casuale;
}

void PRENOTA(int id, int num_persone, Boolean *voli_finiti)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    /* mi inserisco nella coda in base al numero di posti che sto prenotando */
    if (num_persone == 1)   /* prenoto per un passeggero singolo */
    {
        printf("PASSEGGERO_SINGOLO-[Thread%d e identificatore %lu] mi inserisco nella coda dei passeggeri singoli \n", id, pthread_self());

        /* inserisco il mio id nella coda dei passeggeri singoli e incremento il numero di persone singole in attesa */
        coda_passeggeri_singoli[contatore_passeggeri_singoli] = id;
        contatore_passeggeri_singoli++;
    }
    else    /* prenoto per un gruppo di passeggeri */
    {
        printf("PASSEGGERI_GRUPPO-[Thread%d e identificatore %lu] siamo un gruppo di passeggeri di %d persone, ci inseriamo nella coda dei passeggeri in gruppo \n", id, pthread_self(), num_persone);

        /* inserisco il mio id nella coda dei passeggeri in gruppo e incremento il numero di passeggeri in gruppo in attesa */
        coda_passeggeri_gruppo[contatore_passeggeri_gruppo].id = id;
        coda_passeggeri_gruppo[contatore_passeggeri_gruppo].num_persone = num_persone;
        contatore_passeggeri_gruppo++;
    }

    while (passeggeri[id - 1] == PASSEGGERO_NON_SERVITO)
    {
        if (voli_eseguiti == NUM_VOLI)  /* se i voli sono terminati torno a casa */
        {
            *voli_finiti = true; /* indico che i voli sono stati terminati, dunque i passeggeri tornano a casa */
            pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
            return;
        }

        /* mi metto in attesa che il pilota mi/ci faccia salire sull'elicottero */
        pthread_cond_wait(&attesa_imbarco, &mutex);
    }

    if (num_persone == 1)
        printf("PASSEGGERO_SINGOLO-[Thread%d e identificatore %lu] il pilota con id %d mi ha fatto salire sull'elicottero\n", id, pthread_self(), passeggeri[id - 1]);
    else
        printf("PASSEGGERI_GRUPPO-[Thread%d e identificatore %lu] il pilota con id %d ci ha fatto salire sull'elicottero\n", id, pthread_self(), passeggeri[id - 1]);

    while(passeggeri[id - 1] != PASSEGGERO_SERVITO)
    {
        /* l'elicottero e' appena partito, attendo il termine del volo */
        pthread_cond_wait(&attesa_termine, &mutex);
    }

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void IMBARCO(int id, int *num_posti_disponibili, int **id_selezionati, int *contatore_selezionati)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    int i;  /* variabile contatore usata per la selezione del gruppo di passeggeri */
    int j;  /* variabile contatore usata per spostare i passeggeri in gruppo/singoli nella coda */
    Boolean trovato;    /* variabile booleana che indica se e' stato trovato un gruppo di passeggeri che può riempire i posti disponibili in elicottero */

    /* controllo se ci sono gruppi di passeggeri in attesa */
    if (contatore_passeggeri_gruppo > 0)
    {
        trovato = true; /* imposto 'trovato' a true per entrare nel ciclo */
        while (trovato == true) /* ripeto la ricerca fin quando non trovo piu' gruppi di passeggeri in grado di riempire i posti rimanenti */
        {
            /* imposto 'trovato' a false cose' che se la ricerca non ha avuto esito positivo usciro' dal ciclo */
            trovato = false;

            for (i = 0; i < contatore_passeggeri_gruppo; i++)
            {
                /* occupo i posti disponibili inserendo prima i passeggeri in gruppo */
                if (*num_posti_disponibili - coda_passeggeri_gruppo[i].num_persone >= 0)
                {
                    /* faccio salire il gruppo di passeggeri in elicottero, inserendo nel loro stato il mio id pilota e inserendo il loro id nell'array id_selezionati */
                    passeggeri[coda_passeggeri_gruppo[i].id - 1] = id;
                    (*id_selezionati)[*contatore_selezionati] = coda_passeggeri_gruppo[i].id;
                    printf("PILOTA-[Thread%d e identificatore %lu] faccio salire il gruppo di passeggeri con id %d composto da %d persone. Posti ancora disponibili: %d\n", id, pthread_self(), coda_passeggeri_gruppo[i].id, coda_passeggeri_gruppo[i].num_persone, *num_posti_disponibili - coda_passeggeri_gruppo[i].num_persone);

                    /* aumento di uno il numero dei passeggeri/gruppi di passeggeri selezionati per salire sull'elicottero */
                    (*contatore_selezionati)++;

                    /* diminuisco il numero di posti disponibili */
                    *num_posti_disponibili -= coda_passeggeri_gruppo[i].num_persone;

                    /* rimuovo dalla coda il gruppo di passeggeri selezionato */
                    for (j = i; j < contatore_passeggeri_gruppo-1; j++)  /* sposto tutti gli altri gruppi di passeggeri di una posizione in coda */
                        coda_passeggeri_gruppo[j] = coda_passeggeri_gruppo[j+1];

                    /* resetto l'ultima posizione della coda */
                    (coda_passeggeri_gruppo[j]).id = -1;
                    (coda_passeggeri_gruppo[j]).num_persone = 0;

                    /* diminuisco di uno il numero di gruppi di passeggeri in coda */
                    contatore_passeggeri_gruppo--;

                    /* dato che ho trovato un gruppo che riempie i posti rimanenti imposto 'trovato' a true */
                    trovato = true;

                    /* termino la ricerca del gruppo di passeggeri */
                    break;
                }
            }
        }
    }

    while (contatore_passeggeri_singoli > 0 && *num_posti_disponibili > 0)  /* se rimangono ancora posti in elicottero e ci sono passeggeri singoli in coda, occupo i posti rimanenti facendoli salire */
    {
        /* faccio salire il passeggero singolo in elicottero, inserendo nel suo stato il mio id pilota e inserendo il suo id nell'array id_selezionati */
        passeggeri[coda_passeggeri_singoli[0] - 1] = id;
        (*id_selezionati)[*contatore_selezionati] = coda_passeggeri_singoli[0];
        printf("PILOTA-[Thread%d e identificatore %lu] faccio salire il passeggero singolo con id %d. Posti ancora disponibili: %d\n", id, pthread_self(), coda_passeggeri_singoli[0], *num_posti_disponibili - 1);

        /* aumento di uno il numero dei passeggeri/gruppi di passeggeri selezionati per salire sull'elicottero */
        (*contatore_selezionati)++;

        /* diminuisco il numero di posti disponibili */
        (*num_posti_disponibili)--;

        /* rimuovo dalla coda il passeggero selezionato */
        for (j = 0; j < contatore_passeggeri_singoli - 1; j++)  /* sposto tutti gli altri passeggeri di una posizione in coda */
            coda_passeggeri_singoli[j] = coda_passeggeri_singoli[j+1];

        /* resetto l'ultima posizione della coda */
        coda_passeggeri_singoli[j] = -1;

        /* diminuisco di uno il numero dei passeggeri singoli in coda */
        contatore_passeggeri_singoli--;
    }

    printf("PILOTA-[Thread%d e identificatore %lu] elicottero in partenza, numero passeggeri: %d\n", id, pthread_self(), NUM_POSTI - *num_posti_disponibili);

    /* notifico i passeggeri che l'elicottero e' in partenza */
    pthread_cond_broadcast(&attesa_imbarco);

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
}

void VOLO_TERMINATO(int id, int *num_posti_disponibili, int **id_selezionati, int *contatore_selezionati)
{
    pthread_mutex_lock(&mutex); /* simulazione dell'ingresso nella procedure entry di un monitor */

    printf("PILOTA-[Thread%d e identificatore %lu] volo %d-esimo terminato, faccio scendere i passeggeri\n", id, pthread_self(), voli_eseguiti);

    while (*contatore_selezionati > 0)
    {
        /* imposto lo stato dei passeggeri in PASSEGGERO_SERVITO (-2) per indicare che il loro volo e' terminato */
        passeggeri[(*id_selezionati)[*contatore_selezionati -1] - 1] = PASSEGGERO_SERVITO;

        /* libero il posto in elicottero */
        (*id_selezionati)[*contatore_selezionati - 1] = -1;

        /* diminuisco il contatore dell'array id_selezionati */
        (*contatore_selezionati)--;
    }

    /* ripristino il numero di posti disponibili in elicottero pari a NUM_POSTI */
    *num_posti_disponibili = NUM_POSTI;

    /* notifico i passeggeri che il volo e' terminato */
    pthread_cond_broadcast(&attesa_termine);

    pthread_mutex_unlock(&mutex);   /* simulazione del termine di una procedure entry di un monitor */
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
    Boolean voli_finiti; /* variabile booleana che indica se sono terminati i voli e pertanto il passeggero e' costretto a tornare a casa */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    /* inizializzo la variabile voli_terminati a false dato che al momento non so se sono gia' finiti */
    voli_finiti = false;

    printf("PASSEGGERO_SINGOLO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    /* effettuo la prenotazione per un passeggero singolo */
    PRENOTA(*pi, 1, &voli_finiti);

    if (voli_finiti)
        printf("PASSEGGERO_SINGOLO-[Thread%d e identificatore %lu] il pilota con ha terminato i voli per oggi, TORNO A CASA\n", *pi, pthread_self());
    else
        printf("PASSEGGERO_SINGOLO-[Thread%d e identificatore %lu] volo terminato, VADO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiPasseggeriGruppo(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int num_persone;    /* variabile che contiene il numero di persone che compone il gruppo, generato in modo random */
    Boolean voli_finiti; /* variabile booleana che indica se sono terminati i voli e pertanto il gruppo di passeggeri e' costretto a tornare a casa */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    /* inizializzo la variabile voli_terminati a false dato che al momento non sappiamo se sono gia' finiti */
    voli_finiti = false;

    printf("PASSEGGERI_GRUPPO-[Thread%d e identificatore %lu] STIAMO ARRIVANDO\n", *pi, pthread_self());

    /* calcolo in modo random il numero di persone che compongono il gruppo */
    num_persone = mia_random(NUM_POSTI);

    /* mi assicuro che il numero di persone che compongono il gruppo non sia di una sola persona */
    if (num_persone == 1)
        num_persone++;

    /* effettuo la prenotazione per un gruppo di passeggeri */
    PRENOTA(*pi, num_persone, &voli_finiti);

    if (voli_finiti)
        printf("PASSEGGERI_GRUPPO-[Thread%d e identificatore %lu] il pilota ha terminato i voli per oggi, TORNIAMO A CASA\n", *pi, pthread_self());
    else
        printf("PASSEGGERI_GRUPPO-[Thread%d e identificatore %lu] volo terminato, ANDIAMO A CASA\n", *pi, pthread_self());

    /* pthread vuole tornare al padre il valore del suo id */
    *ptr = *pi;

    pthread_exit((void *) ptr);
}

void *eseguiPilota(void *id)
{
    int *pi = (int *) id;
    int *ptr;
    int num_posti_disponibili;  /* variabile che contiene il numero di posti attualmente disponibili per il volo corrente */
    int *id_selezionati; /* array contenente gli id dei passeggeri/gruppi di passeggeri che sono stati selezionati per salire sull'elicottero */
    int contatore_selezionati; /* variabile contatore per l'array id_selezionati */

    ptr = (int *) malloc(sizeof(int));
    if (ptr == NULL)
    {
        perror("Errore: Problemi con l'allocazione di ptr\n");
        exit(-1);
    }

    /* definisco l'array contenente l'id dei passeggeri selezionati per salire sull'elicottero */
    id_selezionati = (int *)  malloc(NUM_POSTI * sizeof(int));
    if (id_selezionati == NULL)
    {
        perror("Errore: Problemi con l'allocazione di id_selezionati\n");
        exit(-1);
    }

    /* inizializzo l'array id_selezionati con tutti gli elementi a -1 (posto libero), nessun passeggero selezionato */
    for(contatore_selezionati = 0; contatore_selezionati < NUM_POSTI; contatore_selezionati++)
        id_selezionati[contatore_selezionati] = -1;

    /* azzero il contatore */
    contatore_selezionati = 0;

    printf("PILOTA-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

    /* inizialmente il numero di posti disponibili sara' pari a NUM_POSTI */
    num_posti_disponibili = NUM_POSTI;

    /* eseguo i voli di questa giornata */
    for (voli_eseguiti = 0; voli_eseguiti < NUM_VOLI; voli_eseguiti++)
    {
        /* attendo che arrivi l'ora di partenza, mediante una sleep di 2 secondi */
        sleep(2);

        /* faccio salire i passeggeri sull'elicottero */
        printf("PILOTA-[Thread%d e identificatore %lu] e' arrivata l'ora di partire, effettuo l'imbarco per il %d-esimo volo\n", *pi, pthread_self(), voli_eseguiti);
        IMBARCO(*pi, &num_posti_disponibili, &id_selezionati, &contatore_selezionati);

        /* simulo il volo mediante una sleep di 4 secondi */
        sleep(4);

        /* termino il volo e faccio scendere i passeggeri */
        VOLO_TERMINATO(*pi, &num_posti_disponibili, &id_selezionati, &contatore_selezionati);
    }

    if (contatore_passeggeri_singoli > 0 || contatore_passeggeri_gruppo > 0)    /* se i voli sono terminati ma ci sono ancora passeggeri in attesa, li sblocco */
    {
        printf("PILOTA-[Thread%d e identificatore %lu] i voli sono finiti ma ci sono ancora passeggeri in coda, li notifico e VADO A CASA\n", *pi, pthread_self());

        /* notifico i passeggeri in attesa */
        pthread_cond_broadcast(&attesa_imbarco);
    }
    else
        printf("PILOTA-[Thread%d e identificatore %lu] voli terminati, VADO A CASA\n", *pi, pthread_self());

    /* dealloco la memoria per l'array id_selezionati non piu' utilizzato */
    free(id_selezionati);

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
        sprintf(error, "Errore: Numero di POSTI insufficienti per l'avvio del programma, minimo 2\n");
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

        /* effettuo una sleep di 1 secondo tra la creazione di un thread e un altro per non riempire sin da subito le code, simulando cosi' un arrivo dei passeggeri in orari differenti */
        sleep(1);
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
