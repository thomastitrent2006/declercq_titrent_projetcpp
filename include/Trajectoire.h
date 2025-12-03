// Trajectoire.h
#ifndef TRAJECTOIRE_H
#define TRAJECTOIRE_H

#include <vector>
#include "Position3D.h"

struct Trajectoire {
    std::vector<Position3D> waypoints;
    double altitudeCible;

    Trajectoire() : altitudeCible(0) {}
};

#endif