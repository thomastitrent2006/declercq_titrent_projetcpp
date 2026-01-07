#ifndef CCR_H
#define CCR_H
#include "Position.h"
#include "ControleurBase.h"
#include "APP.h"
#include <map>
#include <string>

struct Aeroport {
    std::string nom;
    Position position;
    APP* controleurApproche = nullptr;  
    int capaciteMax = 0;                
    int avionsEnApproche = 0;           
};

struct Route {
    std::string depart;
    std::string arrivee;
    double distance = 0.0;
    std::vector<Position> waypoints; 
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
    double calculerSeparationMinimale(const Avion* a1, const Avion* a2) const;

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

    void recupererAvionsEnCroisiere();

    void recevoirAvionDepuisAPP(Avion* avion, const std::string& aeroportDepart);
};

#endif // CCR_H


