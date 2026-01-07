
#include "../include/Avion.h"
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::chrono::steady_clock::time_point Avion::tempsDebutSimulation;
bool Avion::simulationDemarree = false;

Avion::Avion(const std::string& nom, const Position& pos_depart,
    const std::vector<Position>& destinations)
    : nom(nom),
    position(pos_depart),
    positionDepart(pos_depart),  // ← Sauvegarder le départ
    destinationsPossibles(destinations),  // ← Liste des destinations
    vitesse(0.0),
    cap(0.0),
    altitude_cible(10000.0),
    etat(EtatAvion::PARKING),
    enRoute(false),
    tempsRoulageDebut(0.0),
    nombreVols(0),  
    premierVol(true),
    enParking(false),  
    tempsAttenteParking(5) {  

    if (!destinationsPossibles.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, destinationsPossibles.size() - 1);
        destination = destinationsPossibles[dis(gen)];
        cap = calculerCap(destination);

        std::cout << "[" << nom << "] Destination initiale: "
            << "(" << (int)(destination.x / 1000) << ", "
            << (int)(destination.y / 1000) << ") km\n";
    }

    // Paramètres de vol
    vitesse_croisiere = 250.0;
    vitesse_montee = 50.0;
    vitesse_descente = 40.0;
    vitesse_roulage = 5.0;
}



void Avion::demarrer() {
    enRoute = true;

    auto dernierTemps = std::chrono::steady_clock::now();

    while (enRoute) {
        // Calculer le delta temps
        auto maintenant = std::chrono::steady_clock::now();
        auto duree = std::chrono::duration_cast<std::chrono::milliseconds>(maintenant - dernierTemps);
        double dt = duree.count() / 1000.0;  // Convertir en secondes
        dernierTemps = maintenant;

        // Mettre à jour l'avion
        if (dt > 0.0 && dt < 1.0) {  // Limiter dt pour éviter les sauts
            update(dt*3.0);
        }

        // Petite pause pour ne pas surcharger le CPU (60 FPS)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Avion::choisirNouvelleDestination() {
    if (destinationsPossibles.empty()) {
        std::cerr << "[" << nom << "] ERREUR: Aucune destination disponible\n";
        return;
    }

    // Générateur aléatoire
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, destinationsPossibles.size() - 1);

    // Choisir une destination différente de la position actuelle
    Position nouvelleDestination;
    int tentatives = 0;
    do {
        int index = dis(gen);
        nouvelleDestination = destinationsPossibles[index];
        tentatives++;
    } while (position.distanceTo(nouvelleDestination) < 1000.0 && tentatives < 10);

    destination = nouvelleDestination;
    cap = calculerCap(destination);

    std::cout << "[" << nom << "] Nouvelle destination choisie: "
        << "(" << (int)(destination.x / 1000) << ", "
        << (int)(destination.y / 1000) << ") km\n";
}

void Avion::update(double dt) {
    switch (etat) {
    case EtatAvion::PARKING:
        updateParking(dt);  
        break;

    case EtatAvion::ROULAGE_DECOLLAGE:
        updateRoulageDecollage(dt);
        break;

    case EtatAvion::DECOLLAGE:
        updateDecollage(dt);
        break;

    case EtatAvion::MONTEE:
        updateMontee(dt);
        break;

    case EtatAvion::CROISIERE:
        updateCroisiere(dt);
        break;

    case EtatAvion::DESCENTE:
        updateDescente(dt);
        break;

    case EtatAvion::APPROCHE:
        updateApproche(dt);
        break;

    case EtatAvion::ATTERRISSAGE:
        updateAtterrissage(dt);
        break;

    case EtatAvion::ROULAGE_ARRIVEE:
        updateRoulageArrivee(dt);
        break;
    }
}

void Avion::updateParking(double dt) {
    if (!enParking) {
        tempsParkingDebut = std::chrono::steady_clock::now();
        enParking = true;

        if (premierVol) {
            tempsAttenteParking = 5;
            premierVol = false;
           
            std::cout << "[" << nom << "] Premier vol - Attente 5 secondes...\n";
        }
        else {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(10, 20);
            tempsAttenteParking = dis(gen);
            std::cout << "[" << nom << "] Attente " << tempsAttenteParking
                << " secondes avant redecollage...\n";
        }
    }

    auto maintenant = std::chrono::steady_clock::now();
    auto duree = std::chrono::duration_cast<std::chrono::seconds>(
        maintenant - tempsParkingDebut);

    if (duree.count() >= tempsAttenteParking) {
        nombreVols++;

        
        if (nombreVols > 1) {
            choisirNouvelleDestination();
        }

        std::cout << "[" << nom << "] Decollage numero " << nombreVols << "\n";
        setEtat(EtatAvion::ROULAGE_DECOLLAGE);
        enParking = false;
    }
}

void Avion::updateRoulageDecollage(double dt) {
    vitesse = vitesse_roulage * 10.0;
    cap = calculerCap(destination);

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);
    
    tempsRoulageDebut += dt;  // Accumuler le temps

    // Passer en décollage après 0.5 seconde
    if (tempsRoulageDebut > 1) {
        setEtat(EtatAvion::DECOLLAGE);
        tempsRoulageDebut = 0.0;  // Reset pour la prochaine fois
    }
}
void Avion::updateDecollage(double dt) {
    if (vitesse < vitesse_croisiere) {
        vitesse += 10.0 * dt;
    }
   

    position.altitude += vitesse_montee * 0.5 * dt;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    if (position.altitude >= 200) {
        setEtat(EtatAvion::MONTEE);
        altitude_cible = 10000;
    }
}

void Avion::updateMontee(double dt) {
    vitesse = vitesse_croisiere * 10.0;
    
    position.altitude += vitesse_montee * 10.0 * dt;

    cap = calculerCap(destination);

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    if (position.altitude >= altitude_cible) {
        position.altitude = altitude_cible;
        setEtat(EtatAvion::CROISIERE);
    }
}

void Avion::updateCroisiere(double dt) {
    vitesse = vitesse_croisiere * 50.0;
    cap = calculerCap(destination);

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    double distance_restante = distanceVers(destination);

    
    if (distance_restante < 5000.0) {  // 5 km
        vitesse = 0;
        position = destination;
        setEtat(EtatAvion::PARKING);
        return;
    }

    
    if (distance_restante < 100000.0) {  
        setEtat(EtatAvion::DESCENTE);
    }
}
void Avion::updateDescente(double dt) {
    position.altitude -= vitesse_descente * dt;
    if (position.altitude < 0) position.altitude = 0;

    if (vitesse > vitesse_croisiere * 0.7) {
        vitesse -= 3.0 * dt;
    }

    cap = calculerCap(destination);

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    if (position.altitude <= 500) {
        setEtat(EtatAvion::APPROCHE);  
    }
}

void Avion::updateApproche(double dt) {
    position.altitude -= vitesse_descente * 0.8 * dt;
    if (position.altitude < 0) position.altitude = 0;

    if (vitesse > 80.0) {
        vitesse -= 5.0 * dt;
    }

    cap = calculerCap(destination);

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    if (position.altitude <= 30) {
        setEtat(EtatAvion::ATTERRISSAGE);
    }
}

void Avion::updateAtterrissage(double dt) {
    position.altitude -= vitesse_descente * 0.5 * dt;
    if (position.altitude < 0) position.altitude = 0;

    vitesse -= 15.0 * dt;
    if (vitesse < 0) vitesse = 0;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    double distance_destination = distanceVers(destination);
    if (distance_destination < 5000.0) {
        vitesse = 0;
        position = destination;
        setEtat(EtatAvion::PARKING);
        std::cout << "[" << nom << "]  Atterri et stationne \n";
    }
}

void Avion::updateRoulageArrivee(double dt) {
    setEtat(EtatAvion::PARKING);
    vitesse = 0;
}

double Avion::distanceVers(const Position& pos) const {
    double dx = pos.x - position.x;
    double dy = pos.y - position.y;
    return sqrt(dx * dx + dy * dy);
}

double Avion::calculerCap(const Position& cible) const {
    double dx = cible.x - position.x;
    double dy = cible.y - position.y;
    double cap_rad = atan2(dy, dx);
    return cap_rad * 180.0 / M_PI;
}

void Avion::afficherEtat() const {
    std::cout << "[" << nom << "] " << getEtatString() << std::endl;
}


bool Avion::volTermine() const {
    // Arrêter si proche de la destination (< 5 km)
    double distance = distanceVers(destination);
    return distance < 5000.0;
}