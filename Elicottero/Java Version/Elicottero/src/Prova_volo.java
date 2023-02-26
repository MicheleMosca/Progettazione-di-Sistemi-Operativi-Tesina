public class Prova_volo
{
    public static void main(String args[])
    {
        // Variabili di inizializzazione del programma
        int N = 5;	// numero max di persone per volo
        int passeggeri = 15;

        // Creazione del monitor del centro prenotazioni
        Elicottero e = new Elicottero(N);

        if (args.length > 0)
        {
            passeggeri = Integer.parseInt(args[0]);
        }

        int posti = 1;

        // Creo il pilota
        new Pilota(e, "Pilota").start();

        // Creazione dei passeggeri che vogliono volare
        for (int i=0; i < passeggeri; i++)
        {
            int tipo = Math.random() > 0.5? 1:0; // tipo 0 = singolo, tipo 1 = gruppo
            if(tipo == 1)
            {
                posti = (int)(Math.random() * (N - 1) + 2);

            }
            else
            {
                posti = 1;
            }
            new Passeggero(e, "passeggeri "+(i), tipo, posti).start();
        }
    }
}