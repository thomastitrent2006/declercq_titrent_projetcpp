// GenerateurAleatoire.h
#ifndef GENERATEUR_ALEATOIRE_H
#define GENERATEUR_ALEATOIRE_H

#include <vector>
#include <random>
#include "Avion.h"

class GenerateurAleatoire {
private:
    static std::mt19937 rng;

public:
    static void initialiser();
    static Avion* genererAvionAleatoire(int id);
    static std::vector<Avion*> genererAvionsAleatoires(int nombre);
};

#endif