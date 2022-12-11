/***********
 *
 * @author Michele Mosca
 * Esame 15 Gennaio 2014
 */
public class Cliente extends Thread
{

    Banca m;
    String nome;
    int tipo;
    int id;

    public Cliente(Banca m, String nome, int tipo, int id)
    {
        super();
        this.m = m;
        this.nome = nome+"-"+tipo;
        this.tipo = tipo;
        this.id = id;
    }

    public void run()
    {

        m.entraBanca(nome, tipo, id);

        try{
            sleep(400);	// TEMPO visione del contenuto della cassetta
        }catch (InterruptedException ie){}

        m.esciBanca(nome, tipo, id);
    }
}