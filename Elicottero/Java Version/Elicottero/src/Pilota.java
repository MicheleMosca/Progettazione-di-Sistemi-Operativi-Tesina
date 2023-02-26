class Pilota extends Thread
{
    private Elicottero e; //monitor
    private String nome;

    public Pilota(Elicottero e, String nome)
    {
        this.e = e;
        this.nome = nome;
    }

    public void run()
    {
        while(true)
        {
            try {
                Thread.sleep(100); // orario prima del volo
            } catch (InterruptedException ie) {}
            e.imbarco(nome);
            try {
                Thread.sleep(300); // tempo per l'imbarco e il viaggio
            } catch (InterruptedException ie) {}

            e.fine_volo(nome);
        }
    }
}
