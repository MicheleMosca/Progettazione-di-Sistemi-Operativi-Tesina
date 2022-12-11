// Principi di SISTEMI OPERATIVI
// Esame del 10 Dicembre 2010

/**
 * Classe Utente
 * rappresenta il thread dell'utente che vuole volare
 * @author: Michele Mosca
 */
class Passeggero extends Thread
{
    private Elicottero e; //monitor
    private String nome;
    private int tipo;
    private int posti;

    public Passeggero(Elicottero e, String nome, int tipo, int posti)
    {
        this.e = e;
        this.tipo = tipo;
        this.posti = posti;
        this.nome = nome + " tipo " + this.tipo + " Posti: " + this.posti;
    }

    public void run()
    {
        e.prenota(nome, tipo, posti);
    }
}