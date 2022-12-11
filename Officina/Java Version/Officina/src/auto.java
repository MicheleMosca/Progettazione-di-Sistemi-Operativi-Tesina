// PRINCIPI di SISTEMI OPERATIVI
// Esame del 16 Luglio 2010

/**
 * Classe Auto
 * rappresenta il thread dell'automobile che va in officina per il bollino blu o il tagliando
 * @author: Michele Mosca
 */
class auto extends Thread
{
    private officina o; //monitor
    private int controllo;
    private String nome;
    private int numero;

    public auto (officina o, int controllo, int numero, String nome)
    {
        this.o = o;
        this.controllo = controllo;
        this.numero = numero;
        this.nome = nome+": Controllo "+controllo;

    }

    public void run()
    {
        o.entra(controllo, numero, nome);
    }
}
