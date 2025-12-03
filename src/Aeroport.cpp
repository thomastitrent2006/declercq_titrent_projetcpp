// Aeroport.cpp
#include "../include/Aeroport.h"

Aeroport::Aeroport(const std::string& code, const Position3D& pos)
    : code(code), position(pos) {
}

void Aeroport::ajouterPiste(const Piste& piste) {
    pistes.push_back(piste);
}

void Aeroport::ajouterParking(const PositionParking& parking) {
    parkings.push_back(parking);
}

Piste* Aeroport::getPisteLibre() {
    for (auto& piste : pistes) {
        if (piste.estLibre()) {
            return &piste;
        }
    }
    return nullptr;
}

PositionParking* Aeroport::getParkingLibre() {
    for (auto& parking : parkings) {
        if (!parking.estOccupee()) {
            return &parking;
        }
    }
    return nullptr;
}