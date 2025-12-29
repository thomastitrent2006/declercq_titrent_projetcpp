#ifndef APP_H
#define APP_H

#include "ControleurBase.h"
#include <queue>

class TWR; // Forward declaration

class APP : public ControleurBase {
private:
    Position centreAeroport;
    double rayonControle;
    std::queue<std::string> fileAttenteAtterrissage;
    TWR* towerReference;

    void processLogic() override;
    void gererNouvelArrivant(Avion* avion);
    void gererTrajectoires();
    void gererUrgences();
    bool demanderAutorisationAtterrissage(const std::string& avionId);

public:
    APP(const std::string& nom, const Position& centre, double rayon);

    void setTowerReference(TWR* twr) { towerReference = twr; }

    // Affichage console circulaire
    void afficherConsole() const;

    // Assigne une trajectoire circulaire autour de l'aéroport
    void assignerTrajectoireCirculaire(Avion* avion, int niveau);
};

#endif // APP_H