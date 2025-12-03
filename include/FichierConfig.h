// FichierConfig.h
#ifndef FICHIER_CONFIG_H
#define FICHIER_CONFIG_H

#include <string>
#include <vector>
#include "Avion.h"

class FichierConfig {
public:
    static std::vector<Avion*> chargerAvionsDepuisFichier(const std::string& nomFichier);
    static void sauvegarderAvionsDansFichier(const std::string& nomFichier, const std::vector<Avion*>& avions);
};

#endif