// PositionParking.cpp
#include "PositionParking.h"

PositionParking::PositionParking(const std::string& id, const Position3D& pos, double dist)
    : identifiant(id), position(pos), occupee(false), distancePiste(dist) {
}