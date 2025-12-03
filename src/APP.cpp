// APP.cpp
#include "../include/APP.h"

APP::APP(const std::string& id, const std::string& codeAeroport, const Position3D& centre, double rayon)
    : identifiant(id), codeAeroport(codeAeroport), centreZone(centre), rayonZone(rayon) {
}

void APP::ajouterFileAttente(const std::string& codeAvion) {
    fileAttenteAtterrissage.push(codeAvion);
}

std::string APP::getProchainAvion() {
    if (fileAttenteAtterrissage.empty()) {
        return "";
    }
    std::string avion = fileAttenteAtterrissage.front();
    fileAttenteAtterrissage.pop();
    return avion;
}