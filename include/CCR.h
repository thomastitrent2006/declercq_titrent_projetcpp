#ifndef CCR_H
#define CCR_H

#include "ControleurBase.h"
#include "APP.h"
#include <map>
#include <string>

struct Aeroport {
    std::string nom;
    Position position;
    APP* controleurApproche;
    int capaciteMax;          // Nombre max d'avions en approche
    int avionsEnApproche;     // Nombre actuel d'avions
};

struct Route {
    std::string depart;
    std::string arrivee;
    double distance;
    std::vector<Position> waypoints; // Points de passage
};

class CCR : public ControleurBase {
private:
    std::map<std::string, Aeroport> aeroports;
    std::vector<Route> routes;
    double altitudeCroisiere;

    void processLogic() override;
    void gererSeparation();           // Éviter les collisions
    void gererFlux();                 // Réguler le flux vers les aéroports
    void transfererVersAPP();         // Transférer les avions aux APP
    bool verifierCapaciteAeroport(const std::string& aeroportId);
    double calculerSeparationMinimale(Avion* a1, Avion* a2) const;

public:
    CCR(const std::string& nom, double altitude = 10000.0);

    // Gestion des aéroports
    void ajouterAeroport(const std::string& nom, const Position& pos,
        APP* app, int capacite = 5);

    // Gestion des routes
    void ajouterRoute(const std::string& depart, const std::string& arrivee);

    // Créer un vol entre deux aéroports
    void creerVol(const std::string& nomAvion, const std::string& depart,
        const std::string& arrivee);

    // Affichage de l'espace aérien
    void afficherEspaceAerien() const;

    // Vérifier les risques de collision
    std::vector<std::pair<std::string, std::string>> detecterRisquesCollision() const;
};

#endif // CCR_H
````````

double CCR::calculerSeparationMinimale(Avion* a1, Avion* a2) const {
    Position pos1 = a1->getPosition();
    Position pos2 = a2->getPosition();

    return pos1.distance3DTo(pos2);
}