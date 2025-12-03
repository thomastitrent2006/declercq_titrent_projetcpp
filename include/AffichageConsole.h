// AffichageConsole.h
#ifndef AFFICHAGE_CONSOLE_H
#define AFFICHAGE_CONSOLE_H

#include <vector>
#include <string>
#include "Avion.h"
#include "APP.h"
#include "TWR.h"

class AffichageConsole {
public:
    static void afficherPositionsAvions(const std::vector<Avion*>& avions);
    static void afficherAPP(const APP& app, const std::vector<Avion*>& avions);
    static void afficherTWR(const TWR& twr, const std::vector<Avion*>& avions);
    static void afficherSeparateur();
    static void clearScreen();
};

#endif