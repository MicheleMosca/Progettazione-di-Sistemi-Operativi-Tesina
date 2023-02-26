import monitor.*;

public class Banca extends Monitor
{

    int NORM = 0, VIP = 1;		// cliente normale e vip

    int B;						// numero bancari
    int MAXarea;				// numero max clienti in area riservata
    int inArea;					// numero clienti in area riservata

    boolean bancario_libero [];	// bancari liberi

    Cond bancario[];			// coda sospensione bancario senza lavoro
    Cond attesaBancario[];		// coda attesa bancario, 0 normale, 1 vip

    Cond attesaVisione[];		// coda sospensione bancario durante visione contenuto da parte del cliente
    Cond attesaApertura[];		// coda sospensione cliente durante apertura cassetta da parte del bancario

    Cond attesaArea;			// coda per i clienti in attesa di posto nell'area riservata

    public Banca (int B, int MAX)
    {
        this.B = B;
        this.MAXarea = MAX;
        this.inArea = 0;

        bancario_libero = new boolean[B];
        bancario = new Cond[B];
        attesaVisione = new Cond[B];
        attesaApertura = new Cond[B];
        for (int i = 0; i < B; i++){
            bancario_libero[i] = true;
            bancario[i] = new Cond (this);
            attesaVisione[i] = new Cond (this);
            attesaApertura[i] = new Cond (this);
        }

        attesaBancario = new Cond[2];
        attesaBancario[0] = new Cond (this); attesaBancario[1] = new Cond (this);

        attesaArea = new Cond (this);
    }

    public void entraBanca(String nome,int tipo, int id)
    {
        entraMonitor();

        // Qui ricerco un bancario libero
        int trovato = -1;
        while (trovato == -1)
        {
            if (inArea + 1 >= MAXarea) // controllo se c'e' troppa gente nell'area riservata, poi il bancario
            {
                System.out.println(nome + " mi SOSPENDO, troppa gente nell'area riservata");
                attesaArea.Wait();
            }
            else
            {
                for(int i = 0; i < B; i++)
                {		// cerco un bancario libero
                    if (bancario_libero[i] == true)
                    {
                        if ( (tipo == VIP) || (!attesaBancario[VIP].queue()))
                        { 		// prioritˆ cliente vip
                            System.out.println(nome + " ho TROVATO un bancario LIBERO: "+i);
                            trovato = i;
                            bancario_libero[i] = false;
                            inArea++;
                            break;
                        }
                    }
                }
                if (trovato == -1)
                {		// se non trovo un bancario mi sospendo su una delle 2 code
                    System.out.println(nome + " NON TROVATO un bancario LIBERO");
                    if (tipo == VIP)
                    {
                        attesaBancario[VIP].Wait();
                    }
                    else
                    {
                        attesaBancario[NORM].Wait();
                    }
                }
            }
        }

        // Qui ho trovato un bancario che mi serve
        System.out.println(nome + " vado alla CASSETTA e ASPETTO che il BANCARIO la apra");
        bancario[trovato].Signal();
        attesaApertura[trovato].Wait();

        // Qui visiono la cassetta
        System.out.println(nome + " ora VISIONO la CASSETTA");

        esciMonitor();
    }

    public void inizioLavoro(String nome, int id)
    {
        entraMonitor();

        if (! (attesaBancario[NORM].queue() || attesaBancario[VIP].queue()))
        {	// se non ho clienti mi sospendo
            System.out.println(nome + " mi SOSPENDO per mancanza clienti");
            bancario[id].Wait();
        }

        // Qui un cliente mi ha risvegliato
        System.out.println(nome + " SERVO un cliente e APRO la cassetta");
        bancario_libero[id] = false;
        esciMonitor();
    }

    public void fineLavoro(String nome, int id)
    {
        entraMonitor();
        System.out.println(nome + " cassetta APERTA, il cliente puo' visionarla, finito di SERVIRE");
        attesaApertura[id].Signal(); 	// segnalo al cliente che ho aperto al cassetta
        bancario_libero[id] = true;

        if (attesaBancario[VIP].queue()) // sveglio utenti in attesa rispettando priorita'
        {
            attesaBancario[VIP].Signal();
        }
        else
        {
            if (attesaBancario[NORM].queue())
            {
                attesaBancario[NORM].Signal();
            }
        }

        esciMonitor();
    }

    public void esciBanca(String nome,int tipo, int id)
    {
        entraMonitor();

        System.out.println(nome + " FINITO di visionare la CASSETTA, vado a casa");
        inArea -- ;
        attesaArea.Signal(); 	// risveglio quelli bloccati causa numero massimo area riservata

        esciMonitor();
    }
}