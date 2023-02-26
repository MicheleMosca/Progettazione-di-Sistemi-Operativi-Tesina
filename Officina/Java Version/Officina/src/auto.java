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
