// GenerateurAleatoire.cpp
#include "GenerateurAleatoire.h"
#include <ctime>

std::mt19937 GenerateurAleatoire::rng;

void GenerateurAleatoire::initialiser() {
    rng.seed(static_cast<unsigned>(std::time(nullptr)));
}

Avion* GenerateurAleatoire::genererAvionAleatoire(int id) {
    std::uniform_real_distribution<double> distX(-50.0, 50.0);
    std::uniform_real_distribution<double> distY(-50.0, 50.0);
    std::uniform_real_distribution<double> distAlt(1000.0, 3000.0);
    std::uniform_real_distribution<double> distVit(200.0, 500.0);

    std::string code = "AVN" + std::to_string(id);
    Position3D pos(distX(rng), distY(rng), distAlt(rng));
    Vitesse vit(distVit(rng), distVit(rng), 0);

    return new Avion(code, pos, vit);
}

std::vector<Avion*> GenerateurAleatoire::genererAvionsAleatoires(int nombre) {
    std::vector<Avion*> avions;
    for (int i = 1; i <= nombre; ++i) {
        avions.push_back(genererAvionAleatoire(i));
    }
    return avions;
}