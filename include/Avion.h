// Avion.h
#ifndef AVION_H
#define AVION_H

#include "Position.h"
#include <string>
#include <chrono>        
#include <thread>        
#include <cmath>
#include <iostream>
#include <random>

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

    // Contrôle d'exécution
    bool enRoute;

    // Temps pour attente au parking
    std::chrono::steady_clock::time_point tempsParkingDebut;
    double tempsRoulageDebut;

    static std::chrono::steady_clock::time_point tempsDebutSimulation;  
    static bool simulationDemarree;


    // NOUVEAUX MEMBRES pour destinations multiples et cycles
    std::vector<Position> destinationsPossibles;  
    Position positionDepart;  //  pour retenir le départ
    int nombreVols;  //  compteur de vols effectués
    bool premierVol;  // pour distinguer le premier vol

    bool enParking;  
    int tempsAttenteParking;

public:
    // Constructeur
    Avion(const std::string& nom, const Position& pos_depart,
        const std::vector<Position>& destinations);

    // Getters
    std::string getNom() const { return nom; }
    Position getPosition() const { return position; }
    double getVitesse() const { return vitesse; }
    double getAltitude() const { return position.altitude; }
    EtatAvion getEtat() const { return etat; }  
    Position getDestination() const { return destination; }
    double getCap() const { return cap; }

    // Setters
    void setEtat(EtatAvion nouvelEtat) {
        if (etat != nouvelEtat) {  // Afficher seulement si changement réel
            std::cout << "[" << nom << "] " << getEtatString()
                << " -> " << getEtatStringFromEnum(nouvelEtat) << "\n";
        }
        etat = nouvelEtat;
    }

    // Méthodes d'état
    std::string getEtatString() const {
        return getEtatStringFromEnum(etat);
    }

    // Contrôle de l'avion
    void demarrer();  // Démarre la boucle de mise à jour
    void arreter() { enRoute = false; }  // Arrête l'avion

    // Méthode principale de mise à jour
    void update(double dt);  // dt = delta temps en secondes

    // Affichage
    void afficherEtat() const;

    // Vérification fin de vol
    bool volTermine() const;

    void choisirNouvelleDestination();

    static void demarrerSimulation() {  
        tempsDebutSimulation = std::chrono::steady_clock::now();
        simulationDemarree = true;
    }

private:
    // Méthodes internes de gestion du vol
    void updateParking(double dt);
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

    std::string getEtatStringFromEnum(EtatAvion e) const {
        switch (e) {
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
};

#endif // AVION_H