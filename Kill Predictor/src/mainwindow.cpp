#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(ui->pushButton_prediction, SIGNAL(clicked(bool)), this, SLOT(calculer()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::calculer()
{

    std::random_device rd; //genérateur de
    std::mt19937 gen(rd()); //nombres aléatoires

    unsigned int n(setN()); //nombre de PA

    unsigned int hp(ui->spinBox_hp->value()); //PV

    double p(0.60); //réussite de base (mains nues)
    setP(p);

    double damages(1.65);//dommages de base (mains nues)
    setDamages(damages);

    double eB(0); //espérance coups réussis blaster
    double eL(0); //espérance coups réussis lizaro
    double eR(0); //espérance coups réussis lanceroquettes
    double eG(0); //espérance coups réussis grenade
    double eN(0); //espérance coups réussis natamy
    double eM(0); //espérance coups réussis sulfateuse

    unsigned int chargesUsed(0);
    unsigned int shooterPa(ui->shooterPa->value());


    std::vector<double> pBk(n+shooterPa+1); //B compte les coups infligés au blaster
    std::vector<double> pLk(n+shooterPa+1); //L compte les coups infligés au lizaro
    std::vector<double> pNk(n+shooterPa+1); //N compte les coups infligés au natamy
    std::vector<double> pGk(n+shooterPa+1); //G compte les coups infligés à la grenade
    std::vector<double> pRk(n+shooterPa+1); //R compte les coups infligés au lance roquettes
    std::vector<double> pMk(n+shooterPa+1); //M compte les coups infligés à la sulfateuse
    std::vector<double> pYk(n+shooterPa+1); //Y compte les coups infligés par toutes les armes (sauf couteau)

    MainWindow::setWeaponsStuff(pBk, pLk, pNk,
                                pGk, pRk, pMk,
                                pYk, eB, eL, eR, eG, eN, eM,
                                chargesUsed, shooterPa, n,damages,
                                p); //fonction fourre-tout dégeulasse

    int diff(chargesUsed - shooterPa);
    if(diff < 0)
        diff = chargesUsed;
    else if(diff >= 0)
        diff = shooterPa;

    std::vector<double> pXk(n+diff+1,0); //X compte le nombre de coups infligés à mains nues ou au couteau
    std::binomial_distribution<> b(n+diff, p); //X suit la loi binomiale de paramètres B(PA-chargesd'armes, p)
    for(unsigned int i(0) ; i <= 300000; ++i)
        ++pXk[b(gen)];
    for(unsigned int i(0) ; i < pXk.size() ; ++i)
    {
        pXk[i] /= 300000;
    }

    std::vector<double> pZk(n+diff+1,0); // Z = X + Y compte le nombre de coups totaux infligés

    if(pYk[1] == 0)
    {
        pZk = pXk;
    }
    else
    {
        for(unsigned int i(0) ; i < pZk.size() ; ++i)
        {
            pZk[i] = MainWindow::getPxMoreY(i, pXk, pYk); //P(X+Y = k) = somme(i = 0 à k, P(X=i)*P(Y=k-i)).
        }
    }



    unsigned int eZ(eB + eL + eR + eG + eN + (n+diff)*p); //espérance nbre de coups réussis totaux : E(Z) = E(Y) + E(X)
    unsigned int eD(eZ * damages); // espérances des dégâts

    unsigned int neededAttempts(ceil(hp/damages)); //coups nécessaires pour tuer la cible : PV/dommages moyens armes + cac
    double pZ_neededAttempts(0); //P(Z>=k) tq k est le nombre de coups minimum nécessaire pour tuer la cible

    for(unsigned int i(neededAttempts) ; i < pZk.size() ; ++i)
         pZ_neededAttempts += pZk[i];

     QString display = "Vous pouvez espérer réussir " + QString::number(eZ) + " coups et infliger " + QString::number(eD)
             + " points de dégâts en moyenne !\n" + "Vous avez " + QString::number(pZ_neededAttempts * 100) +
             " % de chances de tuer votre cible !";

     QMessageBox::information(this, "Prédiction !",display);

     writeDataInFile(n, hp, eZ, eD, pZ_neededAttempts);


}

double MainWindow::getP(double m, double a)
{
    //an est la probabilité de réussir le n-ième coup en tenant compte des échecs/réussites précédents
    //an est définie par la suite an+1 = an(m + a - m *an), selon les règles des probabilités (arbre de probabilité)
    //m : multiplicateur de la probabilité de réussite d'un coup suite à un échec (25%, 30% si persévérant)
    //a : probabilité de réussir un coup si le précédent est réussi (60% de base, 72% expert etc...)

     double an(a);
     for(unsigned int i(0) ; i <= 1000 ; ++i)
         an = an *(m + a - m*an);
     //on calcule la limite de an pour déterminer la proba de réussir un coup lorsque le nombre de coups est important
     //(concrètement an converge vers le 8/9e coup pour a = 0,60)

     return an;

}

void MainWindow::writeDataInFile(const unsigned int n, const unsigned int hp, const double eX, const double eY,
                                 const double neededAttempts)
{
    std::ofstream file("data.txt", std::ios::app);
    if(!file)
        std::cerr << "Impossible d'ouvrir données.txt !" << std::endl;


    file << "Données - PA : " << n << " ; PV cible : " << hp << " ; ";

    if(ui->pacifist->isChecked())
        file << "Pacifisme, ";
    if(ui->crazyEye->isChecked())
        file << "Oeil Fou, ";
    if(ui->expert->isChecked())
    {
        file << "Expert, ";
    }
    if(ui->elusive->isChecked())
    {
        file << "Fuyant, ";
    }

    if(ui->creative->isChecked())
    {
        file << "Creatif, ";
    }
    if(ui->persistent->isChecked())
    {
        file << "Persévérant, ";
    }

    if(ui->solid->isChecked())
        file << "Robuste, ";
    if(ui->wrestler->isChecked())
        file << "Lutteur, ";
    if(ui->knife->isChecked())
        file << "Couteau, ";
    if(ui->armor->isChecked())
        file << "Armure, ";
    if(ui->roughneck->isChecked())
        file << "Dur-à-cuire, ";

    file << "\nCoups réussis : " << eX << " ; Dégâts infligés : " << eY << " ; Probabilité de meurtre réussi : "
         << neededAttempts << "\n" << std::endl;
}

double MainWindow::getPxMoreY(const unsigned int k, std::vector<double> const &pX, std::vector<double> const &pY)
{
    double val(0);

    //P(X+Y = k) = somme(i = 0 à k, P(X=i)*P(Y=k-i)).
    for(unsigned int i(0) ; i <= k ; ++i)
       val += pX[i]*pY[k-i];



    return val;

}

unsigned int MainWindow::setN()
{
    unsigned int var(ui->spinBox_paNumber->value());

    if(ui->pacifist->isChecked() || ui->crazyEye->isChecked()) //si pacifistes et/ou oeil fou checked, on divise le nbre de PA dispos
         var /= 3;
    else if(ui->pacifist->isChecked() && ui->crazyEye->isChecked())
         var /= 5;

    return var;
}

void MainWindow::setWeaponsStuff(std::vector<double> &pBk, std::vector<double> &pLk, std::vector<double> &pNk,
                                 std::vector<double> &pGk, std::vector<double> &pRk, std::vector<double> &pMk,
                                 std::vector<double> &pYk, double &eB, double &eL, double &eR, double &eG, double &eN,double &eM,
                                 unsigned int &chargesUsed, const unsigned int shooterPa, const unsigned int n, double &damages, const double p)
{

    std::random_device rd; //genérateur de
    std::mt19937 gen(rd()); //nombres aléatoires
    int shootPa(shooterPa);

    if(ui->blaster->isChecked())
    {

        chargesUsed += ui->blasterCharges->value();
        int var(shootPa);
        if(shootPa < 0)
            shootPa = 0;
        else if(shootPa > ui->blasterCharges->value())
            var = ui->blasterCharges->value();


        damages = damages * (n+var-ui->blasterCharges->value())/(n+var)
                + 2.7 * ui->blasterCharges->value()/(n+var);
        eB = ui->blasterCharges->value() * p*5/6; //multiplicateur pour prendre en compte les compétences agissant sur p (expert...)

        //calcul des p(B=k) :
        std::binomial_distribution<> b(ui->blasterCharges->value(), p*5/6);
        for(unsigned int i(0) ; i <= 300000; ++i)
        {
            ++pBk[b(gen)];

        }


        for(unsigned int i(0) ; i < pBk.size() ; ++i)
        {
            pBk[i] /= 300000;
        }

        pYk = pBk; //Y = B+...+armes

        shootPa -= chargesUsed;


    }

    if(ui->lizaroJungle->isChecked())
    {
        chargesUsed += ui->lizaroCharges->value();
        int var2(shootPa);
        if(shootPa < 0)
            shootPa = 0;
        else if(shootPa > ui->lizaroCharges->value())
            var2 = ui->lizaroCharges->value();


        std::vector<double> var(pYk);

        damages = damages * (n+var2-ui->lizaroCharges->value())/(n+var2)
                + 3.65 * ui->lizaroCharges->value()/(n+var2);
        eL = ui->lizaroCharges->value() * 0.99;

        //calcul des p(L=k)
        std::binomial_distribution<> b(ui->lizaroCharges->value(), p*0.99);
        for(unsigned int i(0) ; i <= 300000; ++i)
            ++pLk[b(gen)];
        for(unsigned int i(0) ; i < pLk.size() ; ++i)
        {
            pLk[i] /= 300000;
        }

        if(pYk[0] == 0)//si les P(Y=k) sont nuls
            pYk = pLk;
        else

        {
            for(unsigned int i(0) ; i <= chargesUsed ; ++i)
            {

                pYk[i] = MainWindow::getPxMoreY(i, pLk, var );
            }


        }

        shootPa -= chargesUsed;

    }

    if(ui->grenadesNumber->value() > 0)
    {
        chargesUsed += ui->grenadesNumber->value();
        std::vector<double> var(pYk);

        damages = damages * (n-ui->grenadesNumber->value())/n + 5*ui->grenadesNumber->value()/n;
        eG = ui->grenadesNumber->value();

        pGk[0] = 0;
        pGk[1] = 1;


        if(pYk[0] == 0)
            pYk = pGk;
        else
        {

            for(unsigned int i(0) ; i <= chargesUsed ; ++i)
            {

                   pYk[i] = MainWindow::getPxMoreY(i, pGk, var);
            }
        }

    }

    if(ui->natamy->isChecked())
    {
        chargesUsed += ui->natamyCharges->value();
        int var2(shootPa);
        if(shootPa < 0)
            shootPa = 0;
        else if(shootPa > ui->natamyCharges->value())
            var2 = ui->natamyCharges->value();

        std::vector<double> var(pYk);

        if(ui->mush->isChecked())
            damages = damages * (n+var2-ui->natamyCharges->value())/(n+var2)
                    + 8 * ui->natamyCharges->value()/(n+var2);
        else
            damages = damages * (n+var2-ui->natamyCharges->value())/(n+var2)
                    + 2.65 * ui->natamyCharges->value()/(n+var2);

        eN = ui->natamyCharges->value() * p*5/6;

        //calcul des p(n=k)
        std::binomial_distribution<> b(ui->natamyCharges->value(), p*5/6);
        for(unsigned int i(0) ; i <= 300000; ++i)
            ++pNk[b(gen)];
        for(unsigned int i(0) ; i < pNk.size() ; ++i)
        {
            pNk[i] /= 300000;
        }

        if(pYk[0] == 0)
            pYk = pNk;
        else
        {

            for(unsigned int i(0) ; i <= chargesUsed ; ++i)
            {
                pYk[i] = MainWindow::getPxMoreY(i, pNk, var);
            }
        }


        shootPa -= chargesUsed;
    }
    if(ui->rocketLauncher->isChecked())
    {
        chargesUsed += ui->rocketLauncherCharges->value();
        int var2(shootPa);
        if(shootPa < 0)
            shootPa = 0;
        else if(shootPa > ui->rocketLauncherCharges->value())
            var2 = ui->rocketLauncherCharges->value();
        std::vector<double> var(pYk);

        damages = damages * (n+var2-ui->rocketLauncherCharges->value())/(n+var2)
                + 4.5 *ui->rocketLauncherCharges->value()/(n+var2);
        eR = ui->rocketLauncherCharges->value() * p*5/6;

        //calcul des p(R=k)
        std::binomial_distribution<> b(ui->rocketLauncherCharges->value(), p*5/6);
        for(unsigned int i(0) ; i <= 300000; ++i)
            ++pRk[b(gen)];
        for(unsigned int i(0) ; i < pRk.size() ; ++i)
        {
            pRk[i] /= 300000;
        }


        if(pYk[0] == 0)
            pYk = pRk;
        else
        {

            for(unsigned int i(0) ; i <= chargesUsed ; ++i)
            {
                   pYk[i] = MainWindow::getPxMoreY(i, pRk, var);
            }
        }

        shootPa -= chargesUsed;

    }

    if(ui->machineGun->isChecked())
    {

        chargesUsed += ui->machineGunCharges->value();
        int var(shootPa);
        if(shootPa < 0)
            shootPa = 0;
        else if(shootPa > ui->machineGunCharges->value())
            var = ui->machineGunCharges->value();


        damages = damages * (n+var-ui->machineGunCharges->value())/(n+var)
                + 2.7 * ui->machineGunCharges->value()/(n+var);
        eM = ui->machineGunCharges->value() * p*5/6; //multiplicateur pour prendre en compte les compétences agissant sur p (expert...)

        //calcul des p(M=k) :
        std::binomial_distribution<> b(ui->machineGunCharges->value(), p*5/6);
        for(unsigned int i(0) ; i <= 300000; ++i)
        {
            ++pMk[b(gen)];

        }

     }

}



void MainWindow::setDamages(double &damages)
{
    //compétences de combat et armes : on modifie les dommages possibles
    if(ui->solid->isChecked())
        damages = 2.65;
    else if(ui->wrestler->isChecked())
        damages = 3.65;
    else if(ui->knife->isChecked())
        damages = 2.25;


    if(ui->armor->isChecked() )
        --damages;
    if(ui->roughneck->isChecked())
        --damages;
    if(ui->berserker->isChecked())
        --damages;
}

void MainWindow::setP(double &p)
{
    //si les compétences donnant un bonus/malus à la réussite sont choisies, on modifie la réussite
    if(ui->expert->isChecked())
    {
        p *= 1.20;//+20% de réussite
        p = getP(1.25, p); //getP renvoie la réussite de la compétence en prenant en compte l'augmentation suite aux échecs
    }
    if(ui->elusive->isChecked())
    {
        p *= 0.75;//-25% de réussite
        p = getP(1.25, p);
    }

    if(ui->creative->isChecked())
    {
        p = 2*p/(1+p);//créatif augmente le taux de réussite d'une action répétée jusqu'à ne plus récupérer de PA
        p = getP(1.25, p);
    }
    if(ui->persistent->isChecked())
    {
        p = getP(1.30, p);//persévérant fait augmenter de 30% la réussite d'une action raté au lieu de 25
    }

    p = getP(1.25, p);
}
