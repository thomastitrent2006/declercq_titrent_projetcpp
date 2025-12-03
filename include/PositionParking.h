// PositionParking.h
#ifndef POSITION_PARKING_H
#define POSITION_PARKING_H

#include <string>
#include "Position3D.h"

class PositionParking {
private:
    std::string identifiant;
    Position3D position;
    bool occupee;
    double distancePiste;

public:
    PositionParking(const std::string& id, const Position3D& pos, double dist);

    bool estOccupee() const { return occupee; }
    std::string getIdentifiant() const { return identifiant; }
    double getDistancePiste() const { return distancePiste; }

    void occuper() { occupee = true; }
    void liberer() { occupee = false; }
};

#endif