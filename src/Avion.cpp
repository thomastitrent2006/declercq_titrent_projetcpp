// Avion.cpp
#include "../include/Avion.h"

Avion::Avion(const std::string& code, const Position3D& pos, const Vitesse& vit)
    : code(code), position(pos), vitesse(vit), etat(EtatAvion::EN_ROUTE), carburant(10000.0) {
}