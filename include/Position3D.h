// Position3D.h
#ifndef POSITION3D_H
#define POSITION3D_H

struct Position3D {
    double x;
    double y;
    double altitude;

    Position3D(double x = 0, double y = 0, double alt = 0)
        : x(x), y(y), altitude(alt) {
    }
};

#endif