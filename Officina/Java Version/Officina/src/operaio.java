// SISTEMI OPERATIVI
// Esame del 16 Luglio 2010

/**
 * Classe Operaio
 * rappresenta il thread dell'operaio che lavora in officina
 * @author: Michele Mosca
 */
class operaio extends Thread
{
    private officina o;
    private String nome;
    private int tipo;
    private int id;

    public operaio (officina o, int tipo, String nome, int id)
    {
        this.o = o;
        this.tipo = tipo;
        this.id = id;
        this.nome = nome+":tipo "+tipo;
    }

    public void run()
    {
        while (true)
        {
            o.inizia_controllo(tipo, nome, id);
            try
            {
                sleep((int)Math.random()*100);
            }
            catch (InterruptedException ie) {};
            o.fine_controllo(tipo, nome, id);
        }
    }

}

