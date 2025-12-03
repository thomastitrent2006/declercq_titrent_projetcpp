// TWR.h
#ifndef TWR_H
#define TWR_H

#include <string>
#include <vector>
#include "Position3D.h"
#include "Piste.h"
#include "PositionParking.h"

class TWR {
private:
    std::string identifiant;
    std::string codeAeroport;
    std::vector<Piste> pistes;
    std::vector<PositionParking> parkings;

public:
    TWR(const std::string& id, const std::string& codeAeroport);

    std::string getIdentifiant() const { return identifiant; }
    void ajouterPiste(const Piste& piste);
    void ajouterParking(const PositionParking& parking);
    Piste* getPisteLibre();
    PositionParking* getParkingLibre();
};

#endif