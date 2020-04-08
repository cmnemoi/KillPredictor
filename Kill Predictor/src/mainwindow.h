#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    double getP(double m, double a);
    void writeDataInFile(const unsigned int n, const unsigned int hp, const double eX, const double eY,
                         const double neededAttempts);
    double getPxMoreY(const unsigned int k, const std::vector<double> &pX, const std::vector<double> &pY);
    unsigned int setN();
    void setWeaponsStuff(std::vector<double> &pBk, std::vector<double> &pLk, std::vector<double> &pNk,
                                std::vector<double> &pGk, std::vector<double> &pRk, std::vector<double> &pMk,
                                std::vector<double> &pYk, double &eB, double &eL, double &eR, double &eG, double &eN,double &eM,
                                unsigned int &chargesUsed, const unsigned int shooterPa, const unsigned int n, double &damages, const double p);
    void setDamages(double &damages);
    void setP(double &p);

public slots:
    void calculer();


private:
    Ui::MainWindow *ui;


};

#endif // MAINWINDOW_H
