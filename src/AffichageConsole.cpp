// AffichageConsole.cpp
#include "../include/AffichageConsole.h"
#include <iostream>
#include <iomanip>

void AffichageConsole::afficherPositionsAvions(const std::vector<Avion*>& avions) {
    std::cout << "\n=== POSITIONS DES AVIONS ===\n";
    std::cout << std::left << std::setw(10) << "Code"
        << std::setw(10) << "X"
        << std::setw(10) << "Y"
        << std::setw(10) << "Altitude"
        << std::setw(15) << "Etat" << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (const auto& avion : avions) {
        Position3D pos = avion->getPosition();
        std::string etat;

        switch (avion->getEtat()) {
        case EtatAvion::EN_ROUTE: etat = "EN_ROUTE"; break;
        case EtatAvion::EN_APPROCHE: etat = "EN_APPROCHE"; break;
        case EtatAvion::EN_ATTERRISSAGE: etat = "ATTERRISSAGE"; break;
        case EtatAvion::AU_SOL: etat = "AU_SOL"; break;
        case EtatAvion::EN_DECOLLAGE: etat = "DECOLLAGE"; break;
        }

        std::cout << std::left << std::setw(10) << avion->getCode()
            << std::setw(10) << std::fixed << std::setprecision(2) << pos.x
            << std::setw(10) << pos.y
            << std::setw(10) << pos.altitude
            << std::setw(15) << etat << "\n";
    }
}

void AffichageConsole::afficherAPP(const APP& app, const std::vector<Avion*>& avions) {
    std::cout << "\n=== CONTROLE APPROCHE (APP) - " << app.getIdentifiant() << " ===\n";
    std::cout << "Avions en approche:\n";

    int count = 0;
    for (const auto& avion : avions) {
        if (avion->getEtat() == EtatAvion::EN_APPROCHE) {
            Position3D pos = avion->getPosition();
            std::cout << "  - " << avion->getCode()
                << " (X: " << pos.x << ", Y: " << pos.y
                << ", Alt: " << pos.altitude << ")\n";
            count++;
        }
    }

    if (count == 0) {
        std::cout << "  Aucun avion en approche\n";
    }
}

void AffichageConsole::afficherTWR(const TWR& twr, const std::vector<Avion*>& avions) {
    std::cout << "\n=== TOUR DE CONTROLE (TWR) - " << twr.getIdentifiant() << " ===\n";
    std::cout << "Avions au sol:\n";

    int count = 0;
    for (const auto& avion : avions) {
        if (avion->getEtat() == EtatAvion::AU_SOL) {
            std::cout << "  - " << avion->getCode() << "\n";
            count++;
        }
    }

    if (count == 0) {
        std::cout << "  Aucun avion au sol\n";
    }
}

void AffichageConsole::afficherSeparateur() {
    std::cout << "\n" << std::string(60, '=') << "\n";
}

void AffichageConsole::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}