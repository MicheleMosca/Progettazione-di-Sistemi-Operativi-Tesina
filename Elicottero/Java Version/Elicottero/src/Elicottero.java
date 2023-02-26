import monitor.*;

class Elicottero extends Monitor
{

    private final int N; 		// Numero di posti per ogni volo

    private int occupati;		// Il numero di posti gia' occupati
    private int attesa[];		// Numero di passeggeri in attesa di salire sul volo (0 singoli - 1 gruppi)
    private int in_elicottero; 	// Numero di processi attualmente in volo


    private boolean in_volo;	// Se l'elicottero e' in volo

    private Cond[] coda_attesa; 		// Coda su cui si sospende chi deve aspettare il prossimo volo per salire
    private Cond volo;					// Coda su cui si sospende chi e' in volo

    public Elicottero(int N)
    {
        this.in_volo = false;
        this.N = N;
        this.occupati = 0;
        this.in_elicottero = 0;
        this.attesa = new int[2];
        this.volo = new Cond(this);
        this.coda_attesa = new Cond[2];
        for(int i=0; i<2; i++)
        {
            this.coda_attesa[i] = new Cond(this);
            this.attesa[i] = 0;
        }
    }

    ////////////////////////////////////////////////////////////////////
    //	  PROCEDURE ENTRY DEI PASSEGGERI   						 	  //
    ////////////////////////////////////////////////////////////////////

    public void prenota(String nome, int tipo, int posti)
    {
        entraMonitor();

        // Mi sospendo se:
        // - l'elicottero è in volo (o sta imbarcando)
        // - sono un singolo e ci sono gruppi in coda
        // - i posti sono finiti
        while((this.in_volo) || ((this.occupati + posti) > N)) // || (tipo == 0 && this.coda_attesa[1].queue()))
        {
            this.attesa[tipo]++;
            System.out.println(nome + " mi sospendo");
            this.coda_attesa[tipo].Wait();
            this.attesa[tipo]--;
        }

        System.out.println(nome + " mi imbarco");
        this.occupati = this.occupati + posti;
        System.out.println("occupati = " + this.occupati);
        this.in_elicottero++;

        // Mi sospendo per l'imbarco e il volo
        this.volo.Wait();

        System.out.println(nome + " sono atterrato, scendo");
        this.in_elicottero--;

        // Non risveglio io in pilota perché questo parte comunque una volta passato l'orario

        esciMonitor();
    }

    ////////////////////////////////////////////////////////////////////
    //	  PROCEDURE ENTRY DEL PILOTA							 	  //
    ////////////////////////////////////////////////////////////////////
    public void imbarco(String nome)
    {
        entraMonitor();

        this.in_volo = true;
        System.out.println(nome + " e' ora di partire, chiudo il volo");

        esciMonitor();
    }


    public void fine_volo(String nome)
    {
        entraMonitor();

        System.out.println(nome + " siamo atterrati, i passeggeri iniziano a scedere");
        int temp = this.in_elicottero; //numero di PROCESSI saliti
        for(int i=0; i<temp; i++)
        {
            this.volo.Signal();
        }

        System.out.println(nome + " libero i posti sull'aereo");
        this.occupati = 0;
        this.in_volo = false;

        // Sveglio i passeggeri in attesa (rispettando la priorita': 1 poi 0)
        for(int i=1; i>=0; i--)
        {
            temp = this.attesa[i];
            for(int j=0; j<temp; j++)
            {
                this.coda_attesa[i].Signal();
            }
        }

        esciMonitor();
    }

}
