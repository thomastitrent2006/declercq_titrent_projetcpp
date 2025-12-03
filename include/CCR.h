// CCR.h
#ifndef CCR_H
#define CCR_H

#include <string>
#include "Position3D.h"

class CCR {
private:
    std::string identifiant;
    Position3D centre;
    double rayon;

public:
    CCR(const std::string& id, const Position3D& centre, double rayon);

    std::string getIdentifiant() const { return identifiant; }
};

#endif