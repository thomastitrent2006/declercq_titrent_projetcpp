#include "Avion.h"
#include <iostream>
#include <ctime>

// Fonction de pause simple (sans threads)
void pause(int milliseconds) {
    clock_t start = clock();
    clock_t end = start + (milliseconds * CLOCKS_PER_SEC / 1000);
    while (clock() < end) {
        // Attente active
    }
}

int main() {
    // Création de l'avion
    Position depart(0, 0, 0);
    Position arrivee(100000, 50000, 0);

    Avion avion("AF123", depart, arrivee);

    std::cout << "========================================" << std::endl;
    std::cout << "   SIMULATION DE VOL - " << avion.getNom() << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    double dt = 1.0;
    double temps_total = 0;
    int compteur_affichage = 0;

    while (!avion.volTermine() && temps_total < 3600) {
        avion.update(dt);

        if (compteur_affichage % 10 == 0) {
            std::cout << "Temps: " << temps_total << "s | ";
            avion.afficherEtat();
            std::cout << std::endl;

            // Pause d'1 seconde
            pause(1000);
        }

        temps_total += dt;
        compteur_affichage++;
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "   FIN DE LA SIMULATION" << std::endl;
    std::cout << "   Temps total: " << temps_total << " secondes" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}