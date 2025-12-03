// TWR.cpp
#include "../include/TWR.h"

TWR::TWR(const std::string& id, const std::string& codeAeroport)
    : identifiant(id), codeAeroport(codeAeroport) {
}

void TWR::ajouterPiste(const Piste& piste) {
    pistes.push_back(piste);
}

void TWR::ajouterParking(const PositionParking& parking) {
    parkings.push_back(parking);
}

Piste* TWR::getPisteLibre() {
    for (auto& piste : pistes) {
        if (piste.estLibre()) {
            return &piste;
        }
    }
    return nullptr;
}

PositionParking* TWR::getParkingLibre() {
    for (auto& parking : parkings) {
        if (!parking.estOccupee()) {
            return &parking;
        }
    }
    return nullptr;
}