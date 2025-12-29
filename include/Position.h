#ifndef POSITION_H
#define POSITION_H

#include <cmath>

struct Position {
    double x;      // Position en mètres
    double y;      // Position en mètres
    double altitude; // Altitude en mètres

    Position() : x(0), y(0), altitude(0) {}
    Position(double _x, double _y, double _alt) : x(_x), y(_y), altitude(_alt) {}

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