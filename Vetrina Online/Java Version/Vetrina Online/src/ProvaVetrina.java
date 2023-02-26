public class ProvaVetrina
{

    public static void main(String[] args) {

        int utenti = 10;
        int corrieri = 3;

        Vetrina v = new Vetrina(corrieri);
        for (int j=0; j<corrieri; j++)
        {
            Corriere c = new Corriere(v, "Corriere "+j, j);
            c.start();
        }

        int scatoloni = 0;

        for (int i = 0; i < utenti; i++)
        {
            scatoloni = (int)((Math.random() * 18) + 1);
            // Gli scatoloni devono essere multipli di 2
            if ((scatoloni%2) != 0)
            {
                scatoloni = scatoloni+1;
            }

            Utente u = new Utente(v, scatoloni, "Utente "+i, i);
            u.start();
        }
    }
}