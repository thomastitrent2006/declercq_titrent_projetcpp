// Avion.h
#ifndef AVION_H
#define AVION_H
#include "Position.h"
#include <string>
#include <cmath>
#include <iostream>

// Énumérations
enum class EtatAvion {
    PARKING,
    ROULAGE_DECOLLAGE,
    DECOLLAGE,
    MONTEE,
    CROISIERE,
    DESCENTE,
    APPROCHE,
    ATTERRISSAGE,
    ROULAGE_ARRIVEE
};

// Structure Position

   

class Avion {
private:
    // Identification
    std::string nom;

    // Position et mouvement
    Position position;
    double vitesse;                     // Vitesse actuelle (m/s)
    double cap;                         // Direction (0-360 degrés)
    double altitude_cible;              // Altitude à atteindre

    // Caractéristiques de vol
    double vitesse_croisiere;           // Vitesse de croisière (m/s)
    double vitesse_montee;              // Vitesse verticale en montée (m/s)
    double vitesse_descente;            // Vitesse verticale en descente (m/s)
    double vitesse_roulage;             // Vitesse au sol (m/s)

    // État
    EtatAvion etat;

    // Destination
    Position destination;

public:
    // Constructeur
    Avion(const std::string& nom, const Position& pos_depart, const Position& dest);

    // Getters
    std::string getNom() const { return nom; }
    Position getPosition() const { return position; }
    double getVitesse() const { return vitesse; }
    double getAltitude() const { return position.altitude; }
    EtatAvion getEtat() const { return etat; }

    // Setters
    void setEtat(EtatAvion nouvelEtat) { etat = nouvelEtat; }

    // Méthode principale de mise à jour
    void update(double dt);  // dt = delta temps en secondes

    // Affichage
    void afficherEtat() const;
    std::string getEtatString() const;

    // Vérification fin de vol
    bool volTermine() const;

private:
    // Méthodes internes de gestion du vol
    void updateRoulageDecollage(double dt);
    void updateDecollage(double dt);
    void updateMontee(double dt);
    void updateCroisiere(double dt);
    void updateDescente(double dt);
    void updateApproche(double dt);
    void updateAtterrissage(double dt);
    void updateRoulageArrivee(double dt);

    // Utilitaires
    double distanceVers(const Position& pos) const;
    double calculerCap(const Position& cible) const;
};

#endif // AVION_H