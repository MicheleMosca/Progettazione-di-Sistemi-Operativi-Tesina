/**
 *
 * Prova in itinere 15/12/2014
 * Processo Corriere
 * @author Michele Mosca
 *
 */
public class Corriere extends Thread {

    Vetrina v;
    String nome;
    int id;

    public Corriere(Vetrina v, String nome, int id)
    {
        this.v = v;
        this.nome = nome;
        this.id = id;
    }

    public void run()
    {
        while(true)
        {
            v.parti(id, nome);
            try {
                sleep(5);
            }
            catch (InterruptedException ie) {}
            v.consegna(id, nome);
            // ritorno
            try {
                sleep(5);
            }
            catch (InterruptedException ie) {}
        }
    }
}