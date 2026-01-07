
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
    positionDepart(pos_depart),
    destinationsPossibles(destinations),
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

    // ✅ UTILISER LA MÊME LOGIQUE que choisirNouvelleDestination()
    if (!destinationsPossibles.empty()) {
        // Créer une liste de destinations DIFFÉRENTES de la position de départ
        std::vector<Position> destinationsValides;
        for (const auto& dest : destinationsPossibles) {
            double distance = pos_depart.distanceTo(dest);
            if (distance > 50000.0) {  // > 50 km
                destinationsValides.push_back(dest);
            }
        }

        // Choisir aléatoirement parmi les destinations valides
        if (!destinationsValides.empty()) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, destinationsValides.size() - 1);
            destination = destinationsValides[dis(gen)];
        }
        else {
            // Fallback : choisir la destination la plus éloignée
            double maxDistance = 0.0;
            for (const auto& dest : destinationsPossibles) {
                double distance = pos_depart.distanceTo(dest);
                if (distance > maxDistance) {
                    maxDistance = distance;
                    destination = dest;
                }
            }
        }

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

    // DEBUG : Afficher position actuelle
    std::cout << "[" << nom << "] Position actuelle: ("
        << (int)(position.x / 1000) << ", " << (int)(position.y / 1000) << ") km\n";

    if (destinationsPossibles.size() == 1) {
        destination = destinationsPossibles[0];
        cap = calculerCap(destination);
        std::cout << "[" << nom << "] Nouvelle destination choisie: "
            << "(" << (int)(destination.x / 1000) << ", "
            << (int)(destination.y / 1000) << ") km\n";
        return;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::vector<Position> destinationsValides;

    // DEBUG : Vérifier toutes les destinations
    for (const auto& dest : destinationsPossibles) {
        double distance = position.distanceTo(dest);
        std::cout << "[" << nom << "]   Dest (" << (int)(dest.x / 1000) << ", "
            << (int)(dest.y / 1000) << ") km -> distance = "
            << (int)(distance / 1000) << " km\n";

        if (distance > 50000.0) {  // > 50 km
            destinationsValides.push_back(dest);
            std::cout << "[" << nom << "]     -> VALIDE\n";
        }
        else {
            std::cout << "[" << nom << "]     -> TROP PROCHE\n";
        }
    }

    if (destinationsValides.empty()) {
        std::cout << "[" << nom << "] AUCUNE destination valide ! Choix de la plus eloignee\n";

        double maxDistance = 0.0;
        Position destinationLaPlusLoin;

        for (const auto& dest : destinationsPossibles) {
            double distance = position.distanceTo(dest);
            if (distance > maxDistance) {
                maxDistance = distance;
                destinationLaPlusLoin = dest;
            }
        }

        destination = destinationLaPlusLoin;
        std::cout << "[" << nom << "]   -> Choisie: (" << (int)(destination.x / 1000)
            << ", " << (int)(destination.y / 1000) << ") km (distance = "
            << (int)(maxDistance / 1000) << " km)\n";
    }
    else {
        std::uniform_int_distribution<> dis(0, destinationsValides.size() - 1);
        destination = destinationsValides[dis(gen)];
    }

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
    // ✅ FORCER vitesse = 0 dès l'entrée en parking
    vitesse = 0.0;
    
    if (!enParking) {
        tempsParkingDebut = std::chrono::steady_clock::now();
        enParking = true;
        
        if (premierVol) {
            tempsAttenteParking = 5;
            premierVol = false;
            std::cout << "[" << nom << "] Premier vol - Attente 5 secondes...\n";
        } else {
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
        
        // ✅ RÉINITIALISER tous les paramètres avant décollage
        vitesse = 0.0;
        position.altitude = 0.0;
        altitude_cible = 10000.0;
        
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
        vitesse += 50.0 * dt;  
    }

    position.altitude += vitesse_montee * 5.0 * dt;  

    
    double distance_parcourue = vitesse * dt * 10.0;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    if (position.altitude >= 200) {
        setEtat(EtatAvion::MONTEE);
        altitude_cible = 10000;
    }
}

void Avion::updateMontee(double dt) {
    
    vitesse = vitesse_croisiere * 20.0;  

    
    position.altitude += vitesse_montee * 20.0 * dt;  

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

    
    if (distance_restante < 150000.0) {  
        setEtat(EtatAvion::DESCENTE);
    }
}
void Avion::updateDescente(double dt) {
    position.altitude -= vitesse_descente * 10.0 * dt;
    if (position.altitude < 0) position.altitude = 0;

    if (vitesse > vitesse_croisiere * 0.7) {
        vitesse -= 3.0 * dt;
    }

    cap = calculerCap(destination);

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    double distance_dest = distanceVers(destination);

    
    if (distance_dest < 5000.0) {  
        vitesse = 0;
        position = destination;
        position.altitude = 0;  
        setEtat(EtatAvion::PARKING);
        std::cout << "[" << nom << "]  Atterri et stationne\n";
        return;
    }

    // Debug
    static int compteur = 0;
    if (compteur++ % 60 == 0) {
        std::cout << "[" << nom << "] DESCENTE: altitude=" << (int)position.altitude
            << "m, distance=" << (int)(distance_dest / 1000) << "km\n";
    }

    if (position.altitude <= 1000) {
        setEtat(EtatAvion::APPROCHE);
    }
}

void Avion::updateApproche(double dt) {
    position.altitude -= vitesse_descente * 5.0 * dt;
    if (position.altitude < 0) position.altitude = 0;

    if (vitesse > 80.0) {
        vitesse -= 5.0 * dt;
    }

    cap = calculerCap(destination);

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    double distance_dest = distanceVers(destination);

    // ✅ AJOUT : Si arrivé à destination, aller directement en PARKING
    if (distance_dest < 5000.0) {  // 5 km
        vitesse = 0;
        position = destination;
        position.altitude = 0;
        setEtat(EtatAvion::PARKING);
        std::cout << "[" << nom << "]  Atterri et stationne\n";
        return;
    }

    if (position.altitude <= 100) {
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