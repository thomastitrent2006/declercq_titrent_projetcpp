// Avion.cpp
#include "../include/Avion.h"
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Avion::Avion(const std::string& nom, const Position& pos_depart, const Position& dest)
    : nom(nom), position(pos_depart), destination(dest),
    vitesse(0), cap(0), altitude_cible(0), etat(EtatAvion::PARKING) {

    // Paramètres de vol adaptés pour simulation accélérée
    vitesse_croisiere = 250.0;
    vitesse_montee = 50.0;          // Montée plus rapide
    vitesse_descente = 40.0;        // Descente plus rapide
    vitesse_roulage = 5.0;

    cap = calculerCap(destination);
}

void Avion::update(double dt) {
    switch (etat) {
    case EtatAvion::PARKING:
        etat = EtatAvion::ROULAGE_DECOLLAGE;
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
    vitesse = vitesse_roulage;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Réduit à 50m de roulage pour passer rapidement au décollage
    if (position.x > 50) {
        etat = EtatAvion::DECOLLAGE;
    }
}

void Avion::updateDecollage(double dt) {
    if (vitesse < vitesse_croisiere) {
        vitesse += 10.0 * dt;  // Accélération plus rapide
    }

    position.altitude += vitesse_montee * 0.5 * dt;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Réduit à 200m pour passer vite en montée
    if (position.altitude >= 200) {
        etat = EtatAvion::MONTEE;
        altitude_cible = 10000;
    }
}

void Avion::updateMontee(double dt) {
    vitesse = vitesse_croisiere;
    position.altitude += vitesse_montee * dt;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    if (position.altitude >= altitude_cible) {
        position.altitude = altitude_cible;
        etat = EtatAvion::CROISIERE;
    }
}

void Avion::updateCroisiere(double dt) {
    vitesse = vitesse_croisiere;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Commence la descente à 30km au lieu de 20km
    double distance_restante = distanceVers(destination);
    if (distance_restante < 30000) {
        etat = EtatAvion::DESCENTE;
    }
}

void Avion::updateDescente(double dt) {
    position.altitude -= vitesse_descente * dt;
    if (position.altitude < 0) position.altitude = 0;

    if (vitesse > vitesse_croisiere * 0.7) {
        vitesse -= 3.0 * dt;  // Ralentissement plus rapide
    }

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Passage en approche à 500m au lieu de 1000m
    if (position.altitude <= 500) {
        etat = EtatAvion::APPROCHE;
    }
}

void Avion::updateApproche(double dt) {
    position.altitude -= vitesse_descente * 0.8 * dt;
    if (position.altitude < 0) position.altitude = 0;

    if (vitesse > 80.0) {
        vitesse -= 5.0 * dt;  // Ralentissement plus marqué
    }

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Atterrissage à 30m au lieu de 50m
    if (position.altitude <= 30) {
        etat = EtatAvion::ATTERRISSAGE;
    }
}

void Avion::updateAtterrissage(double dt) {
    position.altitude -= vitesse_descente * 0.5 * dt;
    if (position.altitude < 0) position.altitude = 0;

    vitesse -= 15.0 * dt;  // Freinage plus fort
    if (vitesse < vitesse_roulage) vitesse = vitesse_roulage;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    if (position.altitude == 0 && vitesse <= vitesse_roulage) {
        etat = EtatAvion::ROULAGE_ARRIVEE;
    }
}

void Avion::updateRoulageArrivee(double dt) {
    vitesse = vitesse_roulage;

    double distance_parcourue = vitesse * dt;
    position.x += distance_parcourue * cos(cap * M_PI / 180.0);
    position.y += distance_parcourue * sin(cap * M_PI / 180.0);

    // Arrêt à 50m au lieu de 100m
    double distance_destination = distanceVers(destination);
    if (distance_destination < 50) {
        vitesse = 0;
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
    std::cout << "[" << nom << "] " << getEtatString() << std::endl;
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