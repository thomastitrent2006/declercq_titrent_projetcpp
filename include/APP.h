// APP.h
#ifndef APP_H
#define APP_H

#include <string>
#include <queue>
#include "Position3D.h"

class APP {
private:
    std::string identifiant;
    std::string codeAeroport;
    Position3D centreZone;
    double rayonZone;
    std::queue<std::string> fileAttenteAtterrissage;

public:
    APP(const std::string& id, const std::string& codeAeroport, const Position3D& centre, double rayon);

    std::string getIdentifiant() const { return identifiant; }
    void ajouterFileAttente(const std::string& codeAvion);
    std::string getProchainAvion();
};

#endif