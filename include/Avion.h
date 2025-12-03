// Avion.h
#ifndef AVION_H
#define AVION_H

#include <string>
#include "Position3D.h"
#include "Vitesse.h"
#include "Trajectoire.h"

enum class EtatAvion {
    EN_ROUTE,
    EN_APPROCHE,
    EN_ATTERRISSAGE,
    AU_SOL,
    EN_DECOLLAGE
};

class Avion {
private:
    std::string code;
    Position3D position;
    Vitesse vitesse;
    EtatAvion etat;
    double carburant;
    std::string positionParking;

public:
    Avion(const std::string& code, const Position3D& pos, const Vitesse& vit);

    std::string getCode() const { return code; }
    Position3D getPosition() const { return position; }
    EtatAvion getEtat() const { return etat; }

    void setPosition(const Position3D& pos) { position = pos; }
    void setEtat(EtatAvion e) { etat = e; }
    void setPositionParking(const std::string& p) { positionParking = p; }
};

#endif