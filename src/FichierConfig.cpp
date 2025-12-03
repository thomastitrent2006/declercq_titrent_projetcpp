// FichierConfig.cpp
#include "../include/FichierConfig.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<Avion*> FichierConfig::chargerAvionsDepuisFichier(const std::string& nomFichier) {
    std::vector<Avion*> avions;
    std::ifstream fichier(nomFichier);

    if (!fichier.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier: " << nomFichier << "\n";
        return avions;
    }

    std::string ligne;
    while (std::getline(fichier, ligne)) {
        if (ligne.empty() || ligne[0] == '#') continue; // Ignorer lignes vides et commentaires

        std::istringstream iss(ligne);
        std::string code;
        double x, y, alt, vx, vy, vz;

        if (iss >> code >> x >> y >> alt >> vx >> vy >> vz) {
            Position3D pos(x, y, alt);
            Vitesse vit(vx, vy, vz);
            Avion* avion = new Avion(code, pos, vit);
            avions.push_back(avion);
        }
    }

    fichier.close();
    std::cout << "Chargement de " << avions.size() << " avions depuis " << nomFichier << "\n";
    return avions;
}

void FichierConfig::sauvegarderAvionsDansFichier(const std::string& nomFichier, const std::vector<Avion*>& avions) {
    std::ofstream fichier(nomFichier);

    if (!fichier.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier: " << nomFichier << "\n";
        return;
    }

    fichier << "# Format: CODE X Y ALTITUDE VX VY VZ\n";

    for (const auto& avion : avions) {
        Position3D pos = avion->getPosition();
        fichier << avion->getCode() << " "
            << pos.x << " " << pos.y << " " << pos.altitude << " "
            << "0 0 0\n";  // Vitesses à 0 pour simplifier
    }

    fichier.close();
    std::cout << "Sauvegarde de " << avions.size() << " avions dans " << nomFichier << "\n";
}