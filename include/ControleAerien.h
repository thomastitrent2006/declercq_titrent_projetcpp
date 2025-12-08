#ifndef CONTROLE_AERIEN_H
#define CONTROLE_AERIEN_H

#include "Avion.h"
#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

// Zone géographique de contrôle
struct ZoneControle {
    double x_min, x_max;
    double y_min, y_max;
    double altitude_min, altitude_max;
    
    ZoneControle(double xmin = 0, double xmax = 0, 
                 double ymin = 0, double ymax = 0,
                 double alt_min = 0, double alt_max = 0)
        : x_min(xmin), x_max(xmax), y_min(ymin), y_max(ymax),
          altitude_min(alt_min), altitude_max(alt_max) {}
    
    bool contient(const Position& pos) const {
        return (pos.x >= x_min && pos.x <= x_max &&
                pos.y >= y_min && pos.y <= y_max &&
                pos.altitude >= altitude_min && pos.altitude <= altitude_max);
    }
};

// Classe de base abstraite pour tous les centres de contrôle
class CentreControle {
protected:
    std::string nom;
    ZoneControle zone;
    std::vector<Avion*> avions_surveilles;
    mutable std::mutex mutex_avions;  // Protection thread-safe
    
    // Thread de contrôle
    std::thread thread_controle;
    std::atomic<bool> actif;
    
public:
    CentreControle(const std::string& nom, const ZoneControle& zone)
        : nom(nom), zone(zone), actif(false) {}
    
    virtual ~CentreControle() {
        arreterThread();
    }
    
    // Gestion des threads
    void demarrerThread();
    void arreterThread();
    void boucleControle();
    
    // Méthode principale de mise à jour
    virtual void update(double dt) = 0;
    
    // Gestion des avions
    void ajouterAvion(Avion* avion);
    void retirerAvion(Avion* avion);
    bool estDansZone(const Avion* avion) const;
    
    // Surveillance
    virtual void surveillerTrafic();
    virtual void detecterConflits();
    
    // Getters
    std::string getNom() const { return nom; }
    std::vector<Avion*> getAvionsSurveilles() const { 
        std::lock_guard<std::mutex> lock(mutex_avions);
        return avions_surveilles; 
    }
    
    // Affichage (optionnel)
    virtual void afficherEtat() const;
    
protected:
    // Méthodes utilitaires
    double calculerSeparation(const Avion* a1, const Avion* a2) const;
    bool risqueCollision(const Avion* a1, const Avion* a2, double seuil_distance) const;
};

// ============================================================================
// Tour de Contrôle (TWR) - Gestion au sol et circuits d'aérodrome
// ============================================================================
class TWR : public CentreControle {
private:
    Position position_aeroport;
    bool piste_disponible;
    Avion* avion_sur_piste;  // Avion actuellement sur la piste
    std::vector<Avion*> file_attente_decollage;
    std::vector<Avion*> file_attente_atterrissage;
    
public:
    TWR(const std::string& nom, const Position& pos_aeroport, const ZoneControle& zone);
    
    void update(double dt) override;
    
    // Gestion des autorisations
    bool autoriserDecollage(Avion* avion);
    bool autoriserAtterrissage(Avion* avion);
    void libererPiste();
    
    // Gestion des files d'attente
    void ajouterFileDecollage(Avion* avion);
    void ajouterFileAtterrissage(Avion* avion);
    
    bool getPisteDisponible() const { return piste_disponible; }
    
    // Surveillance spécifique TWR
    void surveillerPiste();
    void surveillerCircuitAerodrome();
    
    // Plus d'afficherEtat override - utilise celui de la classe de base
};

// ============================================================================
// Centre de Contrôle d'Approche (APP) - Gestion des montées/descentes
// ============================================================================
class APP : public CentreControle {
private:
    double altitude_transition_ccr;  // Altitude de transfert vers le CCR
    double altitude_transition_twr;  // Altitude de transfert vers la TWR
    std::vector<Avion*> avions_en_approche;
    std::vector<Avion*> avions_en_depart;
    
public:
    APP(const std::string& nom, const ZoneControle& zone, 
        double alt_ccr = 10000, double alt_twr = 1000);
    
    void update(double dt) override;
    
    // Gestion des transitions
    bool doitTransfererVersCCR(const Avion* avion) const;
    bool doitTransfererVersTWR(const Avion* avion) const;
    
    // Gestion des procédures
    void gererMontees(double dt);
    void gererDescentes(double dt);
    void assignerAltitudeIntermediaire(Avion* avion, double altitude);
    
    // Séquencement
    void sequencerApproches();
    void espacerDeparts();
    
    // Plus d'afficherEtat override - utilise celui de la classe de base
    
private:
    void classerAvions();  // Classe les avions en approche/départ
};

// ============================================================================
// Centre de Contrôle Régional (CCR/ACC) - Gestion en-route
// ============================================================================
class CCR : public CentreControle {
private:
    double altitude_min_croisiere;
    std::vector<std::string> routes_aeriennes;  // Routes définies dans la zone
    
    struct Secteur {
        std::string nom;
        ZoneControle zone;
        int capacite_max;
        int nb_avions_actuels;
    };
    
    std::vector<Secteur> secteurs;
    
public:
    CCR(const std::string& nom, const ZoneControle& zone, double alt_min = 5000);
    
    void update(double dt) override;
    
    // Gestion des secteurs
    void ajouterSecteur(const std::string& nom, const ZoneControle& zone, int capacite);
    Secteur* trouverSecteur(const Position& pos);
    
    // Gestion en-route
    void surveillerCroisiere();
    void optimiserRoutes();
    void gererSeparationVerticale();
    
    // Transferts
    bool doitTransfererVersAPP(const Avion* avion, double distance_destination) const;
    
    // Gestion du trafic
    void verifierCapaciteSecteurs();
    void equilibrerCharge();
    
    // Plus d'afficherEtat override - utilise celui de la classe de base
};

#endif // CONTROLE_AERIEN_H