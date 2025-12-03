// Avion.cpp
#include "Avion.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Avion::Avion(const std::string& nom, const Position& pos_depart, const Position& dest)
    : nom(nom), position(pos_depart), destination(dest),
    vitesse(0), cap(0), altitude_cible(0), etat(EtatAvion::PARKING) {

    // Paramètres de vol typiques
    vitesse_croisiere = 250.0;      // ~900 km/h
    vitesse_montee = 10.0;          // 10 m/s de montée
    vitesse_descente = 8.0;         // 8 m/s de descente
    vitesse_roulage = 5.0;          // ~18 km/h au sol

    // Calculer le cap vers la destination
    cap = calculerCap(destination);
}

void Avion::update(double dt) {
    switch (etat) {
    case EtatAvion::PARKING:
        // Démarrage du vol
        etat = EtatAvion::ROULAGE_DECOLLAGE;
        std::cout << "[" << nom << "] Début du roulage vers la piste" << std::endl;
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

void Avion::updateRoulageDecollage(double dt) {
    // Roulage jusqu'à la piste (simulation simple: 1000m)
    vitesse = vitesse_roulage;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Après 200m de roulage, on passe au décollage
    if (position.x > 200) {
        etat = EtatAvion::DECOLLAGE;
        std::cout << "[" << nom << "] Début du décollage" << std::endl;
    }
}

void Avion::updateDecollage(double dt) {
    // Accélération et début de montée
    if (vitesse < vitesse_croisiere) {
        vitesse += 5.0 * dt;  // Accélération progressive
    }

    // Montée progressive
    position.altitude += vitesse_montee * 0.5 * dt;

    // Déplacement horizontal
    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Passage en montée à 500m d'altitude
    if (position.altitude >= 500) {
        etat = EtatAvion::MONTEE;
        altitude_cible = 10000;  // Altitude de croisière: 10000m
        std::cout << "[" << nom << "] Phase de montée vers " << altitude_cible << "m" << std::endl;
    }
}

void Avion::updateMontee(double dt) {
    // Montée vers l'altitude de croisière
    vitesse = vitesse_croisiere;
    position.altitude += vitesse_montee * dt;

    // Déplacement horizontal
    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Passage en croisière
    if (position.altitude >= altitude_cible) {
        position.altitude = altitude_cible;
        etat = EtatAvion::CROISIERE;
        std::cout << "[" << nom << "] Altitude de croisière atteinte: " << altitude_cible << "m" << std::endl;
    }
}

void Avion::updateCroisiere(double dt) {
    // Vol en ligne droite à altitude constante
    vitesse = vitesse_croisiere;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Vérifier si on approche de la destination (20km)
    double distance_restante = distanceVers(destination);
    if (distance_restante < 20000) {
        etat = EtatAvion::DESCENTE;
        std::cout << "[" << nom << "] Début de la descente" << std::endl;
    }
}

void Avion::updateDescente(double dt) {
    // Descente progressive
    position.altitude -= vitesse_descente * dt;
    if (position.altitude < 0) position.altitude = 0;

    // Ralentissement progressif
    if (vitesse > vitesse_croisiere * 0.7) {
        vitesse -= 2.0 * dt;
    }

    // Déplacement horizontal
    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Passage en approche à 1000m d'altitude
    if (position.altitude <= 1000) {
        etat = EtatAvion::APPROCHE;
        std::cout << "[" << nom << "] Phase d'approche finale" << std::endl;
    }
}

void Avion::updateApproche(double dt) {
    // Approche finale
    position.altitude -= vitesse_descente * 0.8 * dt;
    if (position.altitude < 0) position.altitude = 0;

    // Ralentissement
    if (vitesse > 80.0) {
        vitesse -= 3.0 * dt;
    }

    // Déplacement
    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Atterrissage à 50m d'altitude
    if (position.altitude <= 50) {
        etat = EtatAvion::ATTERRISSAGE;
        std::cout << "[" << nom << "] Atterrissage en cours" << std::endl;
    }
}

void Avion::updateAtterrissage(double dt) {
    // Toucher des roues
    position.altitude -= vitesse_descente * 0.5 * dt;
    if (position.altitude < 0) position.altitude = 0;

    // Freinage
    vitesse -= 10.0 * dt;
    if (vitesse < vitesse_roulage) vitesse = vitesse_roulage;

    // Déplacement
    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Au sol et vitesse réduite
    if (position.altitude == 0 && vitesse <= vitesse_roulage) {
        etat = EtatAvion::ROULAGE_ARRIVEE;
        std::cout << "[" << nom << "] Atterrissage réussi, roulage vers le parking" << std::endl;
    }
}

void Avion::updateRoulageArrivee(double dt) {
    // Roulage vers le parking
    vitesse = vitesse_roulage;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Simulation: après 500m de roulage, on arrive au parking
    double distance_destination = distanceVers(destination);
    if (distance_destination < 100) {
        vitesse = 0;
        std::cout << "[" << nom << "] Arrivée au parking. Vol terminé." << std::endl;
    }
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
    std::cout << "[" << nom << "] " << getEtatString()
        << " | Vitesse: " << vitesse << " m/s"
        << " | ";
    position.afficher();
}

std::string Avion::getEtatString() const {
    switch (etat) {
    case EtatAvion::PARKING: return "PARKING";
    case EtatAvion::ROULAGE_DECOLLAGE: return "ROULAGE_DECOLLAGE";
    case EtatAvion::DECOLLAGE: return "DECOLLAGE";
    case EtatAvion::MONTEE: return "MONTEE";
    case EtatAvion::CROISIERE: return "CROISIERE";
    case EtatAvion::DESCENTE: return "DESCENTE";
    case EtatAvion::APPROCHE: return "APPROCHE";
    case EtatAvion::ATTERRISSAGE: return "ATTERRISSAGE";
    case EtatAvion::ROULAGE_ARRIVEE: return "ROULAGE_ARRIVEE";
    default: return "INCONNU";
    }
}

bool Avion::volTermine() const {
    return (etat == EtatAvion::ROULAGE_ARRIVEE && vitesse == 0);
}