/**
 * Prova in itinere 15/12/2014
 * Processo Utente
 * @author Michele Mosca
 *
 */
public class Utente extends Thread
{

    Vetrina v;
    int scatoloni;
    String nome;
    int id;

    public Utente(Vetrina v, int scatoloni, String nome, int id)
    {
        this.v = v;
        this.scatoloni = scatoloni;
        this.nome = nome + " ordina "+scatoloni + " scatoloni";
        this.id = id;
    }

    public void run()
    {
        v.ordina(scatoloni, nome, id);
    }
}