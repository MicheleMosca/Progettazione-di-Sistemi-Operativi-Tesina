public class Bancario extends Thread
{

    Banca m;
    String nome;
    int id;

    public Bancario(Banca m, String nome, int id)
    {
        super();
        this.m = m;
        this.nome = nome;
        this.id = id;
    }

    public void run()
    {

        while (true)
        {
            m.inizioLavoro(nome,id);

            try{
                sleep(300);	// TEMPO apertura cassetta di sicurezza
            }catch (InterruptedException ie){}

            m.fineLavoro(nome, id);
        }
    }
}