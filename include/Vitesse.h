// Vitesse.h
#ifndef VITESSE_H
#define VITESSE_H

struct Vitesse {
    double vx;
    double vy;
    double vz;

    Vitesse(double vx = 0, double vy = 0, double vz = 0)
        : vx(vx), vy(vy), vz(vz) {
    }

    double getNorme() const; // <-- déclaration manquante ajoutée
};

#endif
