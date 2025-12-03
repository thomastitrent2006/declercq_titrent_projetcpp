// Aeroport.h
#ifndef AEROPORT_H
#define AEROPORT_H

#include <string>
#include <vector>
#include "Position3D.h"
#include "Piste.h"
#include "PositionParking.h"

class Aeroport {
private:
    std::string code;
    Position3D position;
    std::vector<Piste> pistes;
    std::vector<PositionParking> parkings;

public:
    Aeroport(const std::string& code, const Position3D& pos);

    void ajouterPiste(const Piste& piste);
    void ajouterParking(const PositionParking& parking);
    Piste* getPisteLibre();
    PositionParking* getParkingLibre();
};

#endif