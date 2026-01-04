// Avion.cpp - VERSION CORRIGÉE
#include "../include/Avion.h"
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Avion::Avion(const std::string& nom, const Position& pos_depart, const Position& dest)
    : nom(nom),
    position(pos_depart),
    destination(dest),
    vitesse(0.0),
    cap(0.0),
    altitude_cible(dest.altitude),
    etat(EtatAvion::PARKING),
    enRoute(false),
    tempsRoulageDebut(0.0) { 

    // Calculer le cap vers la destination DÈS LE DÉBUT
    cap = calculerCap(destination);  // ← AJOUTÉ : Calcul du cap initial

    // Paramètres de vol adaptés pour simulation accélérée
    vitesse_croisiere = 250.0;
    vitesse_montee = 50.0;          // Montée plus rapide
    vitesse_descente = 40.0;        // Descente plus rapide
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
            update(dt);
        }

        // Vérifier si le vol est terminé
        if (volTermine()) {
            enRoute = false;
            break;
        }

        // Petite pause pour ne pas surcharger le CPU (60 FPS)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Avion::update(double dt) {
    switch (etat) {
    case EtatAvion::PARKING:
        updateParking(dt);  // ← Nouvelle méthode
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
    // Si c'est la première fois en parking, noter l'heure
    static bool premiereEntreeParking = true;

    if (premiereEntreeParking) {
        tempsParkingDebut = std::chrono::steady_clock::now();
        premiereEntreeParking = false;
        std::cout << "[" << nom << "] Attente au parking pendant 5 secondes...\n";
    }

    // Vérifier si 5 secondes se sont écoulées
    auto maintenant = std::chrono::steady_clock::now();
    auto duree = std::chrono::duration_cast<std::chrono::seconds>(maintenant - tempsParkingDebut);

    if (duree.count() >= 5) {
        setEtat(EtatAvion::ROULAGE_DECOLLAGE);
        premiereEntreeParking = true;  // Reset pour le prochain parking
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

    if (distance_restante < 5000.0) {
        vitesse = 0;
        position = destination;
        setEtat(EtatAvion::PARKING);
        return;
    }

    if (distance_restante < 30000) {
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