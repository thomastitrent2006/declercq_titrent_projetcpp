#ifndef POSITION_H
#define POSITION_H

#include <cmath>

struct Position {
    double x;           // Coordonnée X (en mètres ou km)
    double y;           // Coordonnée Y (en mètres ou km)
    double altitude;    // Altitude (en mètres)

    Position(double x = 0, double y = 0, double altitude = 0)
        : x(x), y(y), altitude(altitude) {
    }


    // Calcule la distance 2D entre deux positions
    double distanceTo(const Position& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Calcule la distance 3D entre deux positions
    double distance3DTo(const Position& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        double dz = altitude - other.altitude;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }
};

#endif // POSITION_H