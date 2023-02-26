public class ProvaBanca
{
    public static void main (String args[])
    {

        int B = 2; 			// banchieri
        int MAX = 4;		// MAX clienti

        int Clienti = 8;	// clienti della banca

        Banca m = new Banca (B, MAX);

        // processi bancario
        for(int i = 0; i < B; i++)
            new Bancario(m, "Bancario["+i+"]", i).start();

        // processi clienti
        for(int i = 0; i < Clienti; i++)
            new Cliente(m, "Cliente["+i+"]", Math.random() > 0.5? 0:1, i).start();
    }


}