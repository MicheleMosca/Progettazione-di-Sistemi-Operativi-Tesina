import monitor.*;

class officina extends Monitor
{

    private final static int BOLLINO = 0;
    private final static int TAGLIANDO = 1;
    private final int O; 				// Numero di operai nell'officina

    private int[] op_operaio;
    private int[] tipo_operaio;

    private boolean[] libero;

    private Cond[] coda_tagliando; 		// Coda su cui si sospendono le auto che stanno facendo il tagliando dagli operi o di tipo 0 o di tipo 1
    private Cond[] coda_fuori; 			// coda su cui sospendere le auto in attesa di essere servite
    // ho una coda per ogni operazione 0-1
    private Cond[] coda_servita;
    private Cond[] operaio;

    public officina(int operai)
    {
        O = operai;
        coda_fuori = new Cond[2];
        coda_tagliando = new Cond[2];
        operaio = new Cond[O];
        coda_servita = new Cond[O];
        op_operaio = new int[O];
        tipo_operaio = new int[O];
        libero = new boolean[O];
        for(int t=0; t<2; t++)
        {
            coda_fuori[t] = new Cond(this);
            coda_tagliando[t] = new Cond(this);
        }
        for(int i=0; i<O; i++)
        {
            libero[i] = false;
            tipo_operaio[i] = -1;
            op_operaio[i] = -1;
            operaio[i] = new Cond(this);
            coda_servita[i] = new Cond(this);
        }
    }

    ////////////////////////////////////////////////////////////
    //	  PROCEDURE ENTRY DELL'OPERAIO  		  			  //
    ////////////////////////////////////////////////////////////

    public void inizia_controllo(int tipo, String nome, int id) 
    {
        entraMonitor();
        tipo_operaio[id] = tipo;

        libero[id] = true;

        // Risveglio le auto in attesa per il controllo per segnalare loro l'evento
        // devo rispettare le priorita'
        if(tipo == 0)
        {
            if(coda_fuori[BOLLINO].queue())
            {
                coda_fuori[BOLLINO].Signal();
            }
            else
            {
                if(coda_fuori[TAGLIANDO].queue())
                {
                    coda_fuori[TAGLIANDO].Signal();
                }
                else
                {
                    System.out.println(nome + " non ci sono auto in coda mi sospendo");
                    operaio[id].Wait();
                }
            }
        }
        else
        {
            if(coda_fuori[TAGLIANDO].queue())
            {
                coda_fuori[TAGLIANDO].Signal();
            }
            else
            {
                System.out.println(nome + " non ci sono auto in coda mi sospendo");
                operaio[id].Wait();
            }
        }

        System.out.println(nome+": effettuo il controllo");
        esciMonitor();
    }

    public void fine_controllo(int tipo, String nome, int id) {
        entraMonitor();

        System.out.println(nome+": ho finito il controllo, libero l'auto.");
        coda_servita[id].Signal();

        esciMonitor();
    }

    ////////////////////////////////////////////////////////////
    //				PROCEDURE ENTRY DELL'AUTO				  //
    ////////////////////////////////////////////////////////////

    public void entra(int controllo, int numero, String nome)
    {
        entraMonitor();

        int id_trovato = -1;
        //Operaio di tipo 0 fa sia BOLLINI che TAGLIANDI
        //Operaio di tipo 1 fa solo TAGLIANDI
        while(id_trovato == -1)
        {
            if(controllo == 0)
            {
                for(int i = 0; i < O; i++)
                {
                    if(tipo_operaio[i] == 0)
                    {
                        if(libero[i] == true)
                        {
                            id_trovato = i;
                        }
                    }
                }
            }
            else
            {
                for(int i = 0; i < O; i++)
                {
                    if(libero[i] == true)
                    {
                        id_trovato = i;
                    }
                }
            }
            if(id_trovato == -1)
            {
                System.out.println(nome + " tutti gli operai stanno lavorando, mi sospendo");
                coda_fuori[controllo].Wait();
            }
        }

        op_operaio[id_trovato] = controllo;
        System.out.println(nome + " ho trovato un operaio libero: "+id_trovato+" faccio il controllo");
        libero[id_trovato] = false;
        operaio[id_trovato].Signal();

        // Mi sospendo in coda dall'operaio che mi sta servendo
        coda_servita[id_trovato].Wait();

        System.out.println(nome + " ho finito");

        esciMonitor();
    }

}





