import monitor.*;

public class Vetrina extends Monitor
{

    /* Variabili di stato del monitor */
    int C;
    boolean corriereLibero;
    int tipo;
    int corrieri[];

    /* Variabili condition del monitor */
    Cond attesaConsegna;
    Cond corriere[];
    Cond attesaCorriere[];

    public Vetrina(int C)
    {
        this.C = C;
        this.corriereLibero  = true;
        corrieri = new int[C];

        this.attesaConsegna = new Cond(this);
        this.attesaCorriere = new Cond[2];
        this.corriere = new Cond[this.C];
        this.attesaCorriere[0] = new Cond(this);
        this.attesaCorriere[1] = new Cond(this);
        for(int i=0; i<this.C; i++)
        {
            this.corrieri[i] = -1;
            this.corriere[i] = new Cond(this);
        }
    }

    public void ordina(int scatoloni, String nome, int id)
    {
        entraMonitor();

        int tipo = 0;
        if (scatoloni != 18)
        {
            tipo = 1;
        }
        int trovato = -1;

        while(trovato == -1)
        {
            for(int i=0; i<this.C; i++)
            {
                if(this.corrieri[i] == -1)
                {
                    // Ho trovato un corriere libero
                    this.corrieri[i] = id;
                    trovato = i;
                    System.out.println(nome + " ho trovato libero il corrire " +i);
                    break;
                }
            }
            if(trovato == -1)
            {
                // Nessun corriere e' libero
                System.out.println(nome + " mi sospendo perche' non ci sono corrieri liberi");
                this.attesaCorriere[tipo].Wait();
            }
        }
        if(corriere[trovato].queue())
        {
            corriere[trovato].Signal();
        }
        System.out.println(nome + " mi sta servendo il corriere "+ trovato);
        this.attesaConsegna.Wait();

        System.out.println(nome + " sono arrivati i miei scatoloni, li pago e vado a casa");

        esciMonitor();
    }


    public void parti(int id, String nome)
    {
        entraMonitor();

        if(!this.attesaCorriere[0].queue() && !this.attesaCorriere[1].queue())
        {
            System.out.println(nome + " non ci sono utenti che ordinano, mi sospendo");
            this.corriere[id].Wait();
        }
        else
        {
            if(this.attesaCorriere[0].queue())
            {
                this.attesaCorriere[0].Signal();
            }
            else
            {
                this.attesaCorriere[1].Signal();
            }
        }

        System.out.println(nome + " sto servendo un utente");

        esciMonitor();
    }
    public void consegna(int id, String nome)
    {
        entraMonitor();

        this.attesaConsegna.Signal();
        System.out.println(nome + " ho consegnato, ora rientro");
        this.corrieri[id] = -1;

        esciMonitor();
    }

}
