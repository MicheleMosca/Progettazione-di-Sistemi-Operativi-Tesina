/**
 * Esame 16 Luglio 2010
 * Class provaofficina
 * contiente il main
 * @author Michele Mosca
 *
 */

class provaofficina
{

    public static void main(String args[])
    {
        // Variabili di inizializzazione del programma
        int nAuto = 10;
        int operai = 6;			// N¡ addetti

        // Creazione del monitor dell'officina
        officina o = new officina(operai);

        if (args.length > 0)
        {
            nAuto = Integer.parseInt(args[0]);
        }

        // Istanziazione degli operai
        // creo due operai di tipo 0 e uno di tipo 1
        for(int i=0; i<operai; i++)
        {
            new operaio(o, 0, "Operiaio "+(i), i).start();
            new operaio(o, 0, "Operiaio "+(i+1), i+1).start();
            new operaio(o, 1, "Operiaio "+(i+2), i+2).start();
            i = i+2;
        }

        // Istanziazione delle auto
        for (int i=0; i < nAuto; i++)
        {
            new auto(o, (Math.random() > 0.5? 1:0), i, "Auto "+(i)).start();
        }
    }


}
