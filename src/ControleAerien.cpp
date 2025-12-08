#include "ControleAerien.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <thread>
#include <mutex>

// Mutex global pour synchroniser les affichages console
std::mutex console_mutex;

// ============================================================================
// CentreControle - Classe de base
// ============================================================================

void CentreControle::ajouterAvion(Avion* avion) {
    std::lock_guard<std::mutex> lock(mutex_avions);
    if (avion && std::find(avions_surveilles.begin(), avions_surveilles.end(), avion) == avions_surveilles.end()) {
        avions_surveilles.push_back(avion);
    }
}

void CentreControle::retirerAvion(Avion* avion) {
    std::lock_guard<std::mutex> lock(mutex_avions);
    auto it = std::find(avions_surveilles.begin(), avions_surveilles.end(), avion);
    if (it != avions_surveilles.end()) {
        avions_surveilles.erase(it);
    }
}

bool CentreControle::estDansZone(const Avion* avion) const {
    return zone.contient(avion->getPosition());
}

void CentreControle::surveillerTrafic() {
    std::lock_guard<std::mutex> lock(mutex_avions);
    for (auto it = avions_surveilles.begin(); it != avions_surveilles.end();) {
        if (!estDansZone(*it)) {
            it = avions_surveilles.erase(it);
        } else {
            ++it;
        }
    }
}

void CentreControle::detecterConflits() {
    std::lock_guard<std::mutex> lock(mutex_avions);
    const double SEPARATION_MIN_HORIZONTALE = 5000;
    const double SEPARATION_MIN_VERTICALE = 300;
    
    for (size_t i = 0; i < avions_surveilles.size(); i++) {
        for (size_t j = i + 1; j < avions_surveilles.size(); j++) {
            if (risqueCollision(avions_surveilles[i], avions_surveilles[j], SEPARATION_MIN_HORIZONTALE)) {
                double sep_vert = std::abs(avions_surveilles[i]->getAltitude() - 
                                          avions_surveilles[j]->getAltitude());
                
                if (sep_vert < SEPARATION_MIN_VERTICALE) {
                    std::lock_guard<std::mutex> console_lock(console_mutex);
                    std::cout << "[" << nom << "] ⚠️  CONFLIT: " 
                              << avions_surveilles[i]->getNom() << " / " 
                              << avions_surveilles[j]->getNom() << std::endl;
                }
            }
        }
    }
}

double CentreControle::calculerSeparation(const Avion* a1, const Avion* a2) const {
    Position p1 = a1->getPosition();
    Position p2 = a2->getPosition();
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    return sqrt(dx * dx + dy * dy);
}

bool CentreControle::risqueCollision(const Avion* a1, const Avion* a2, double seuil_distance) const {
    return calculerSeparation(a1, a2) < seuil_distance;
}

void CentreControle::afficherEtat() const {
    std::lock_guard<std::mutex> lock1(mutex_avions);
    std::lock_guard<std::mutex> lock2(console_mutex);
    
    std::cout << "\n=== " << nom << " ===" << std::endl;
    std::cout << "Avions: " << avions_surveilles.size() << std::endl;
}

void CentreControle::demarrerThread() {
    actif = true;
    thread_controle = std::thread(&CentreControle::boucleControle, this);
}

void CentreControle::arreterThread() {
    actif = false;
    if (thread_controle.joinable()) {
        thread_controle.join();
    }
}

void CentreControle::boucleControle() {
    while (actif) {
        update(1.0);  // Update toutes les secondes
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// ============================================================================
// TWR - Tour de Contrôle
// ============================================================================

TWR::TWR(const std::string& nom, const Position& pos_aeroport, const ZoneControle& zone)
    : CentreControle(nom, zone), position_aeroport(pos_aeroport), 
      piste_disponible(true), avion_sur_piste(nullptr) {}

void TWR::update(double dt) {
    surveillerTrafic();
    surveillerPiste();
    detecterConflits();
    
    // Gérer les décollages
    if (piste_disponible && !file_attente_decollage.empty()) {
        Avion* avion = file_attente_decollage.front();
        if (autoriserDecollage(avion)) {
            file_attente_decollage.erase(file_attente_decollage.begin());
        }
    }
    
    // Gérer les atterrissages
    if (piste_disponible && !file_attente_atterrissage.empty()) {
        Avion* avion = file_attente_atterrissage.front();
        if (autoriserAtterrissage(avion)) {
            file_attente_atterrissage.erase(file_attente_atterrissage.begin());
        }
    }
}

bool TWR::autoriserDecollage(Avion* avion) {
    if (piste_disponible && avion->getEtat() == EtatAvion::ROULAGE_DECOLLAGE) {
        piste_disponible = false;
        avion_sur_piste = avion;
        return true;
    }
    return false;
}

bool TWR::autoriserAtterrissage(Avion* avion) {
    if (piste_disponible && avion->getEtat() == EtatAvion::APPROCHE) {
        piste_disponible = false;
        avion_sur_piste = avion;
        return true;
    }
    return false;
}

void TWR::libererPiste() {
    if (avion_sur_piste && 
        (avion_sur_piste->getEtat() == EtatAvion::ROULAGE_ARRIVEE || 
         avion_sur_piste->getEtat() == EtatAvion::MONTEE)) {
        piste_disponible = true;
        avion_sur_piste = nullptr;
    }
}

void TWR::surveillerPiste() {
    libererPiste();
}

void TWR::ajouterFileDecollage(Avion* avion) {
    file_attente_decollage.push_back(avion);
}

void TWR::ajouterFileAtterrissage(Avion* avion) {
    file_attente_atterrissage.push_back(avion);
}

// ============================================================================
// APP - Centre de Contrôle d'Approche
// ============================================================================

APP::APP(const std::string& nom, const ZoneControle& zone, double alt_ccr, double alt_twr)
    : CentreControle(nom, zone), altitude_transition_ccr(alt_ccr), altitude_transition_twr(alt_twr) {}

void APP::update(double dt) {
    surveillerTrafic();
    classerAvions();
    
    std::lock_guard<std::mutex> lock(mutex_avions);
    
    // Affichage uniquement pour les avions en MONTEE
    for (auto& avion : avions_en_depart) {
        if (avion->getEtat() == EtatAvion::MONTEE) {
            std::lock_guard<std::mutex> console_lock(console_mutex);
            std::cout << "[" << nom << "] " << avion->getNom() 
                      << " en MONTEE (" << (int)avion->getAltitude() << "m)" << std::endl;
        }
    }
    
    detecterConflits();
}

void APP::classerAvions() {
    std::lock_guard<std::mutex> lock(mutex_avions);
    avions_en_approche.clear();
    avions_en_depart.clear();
    
    for (auto& avion : avions_surveilles) {
        EtatAvion etat = avion->getEtat();
        if (etat == EtatAvion::DESCENTE || etat == EtatAvion::APPROCHE) {
            avions_en_approche.push_back(avion);
        } else if (etat == EtatAvion::MONTEE) {
            avions_en_depart.push_back(avion);
        }
    }
}

bool APP::doitTransfererVersCCR(const Avion* avion) const {
    return avion->getAltitude() >= altitude_transition_ccr && 
           avion->getEtat() == EtatAvion::MONTEE;
}

bool APP::doitTransfererVersTWR(const Avion* avion) const {
    return avion->getAltitude() <= altitude_transition_twr && 
           avion->getEtat() == EtatAvion::APPROCHE;
}

// ============================================================================
// CCR - Centre de Contrôle Régional
// ============================================================================

CCR::CCR(const std::string& nom, const ZoneControle& zone, double alt_min)
    : CentreControle(nom, zone), altitude_min_croisiere(alt_min) {}

void CCR::update(double dt) {
    surveillerTrafic();
    
    std::lock_guard<std::mutex> lock(mutex_avions);
    
    // Affichage uniquement pour les avions en CROISIERE
    for (auto& avion : avions_surveilles) {
        if (avion->getEtat() == EtatAvion::CROISIERE) {
            std::lock_guard<std::mutex> console_lock(console_mutex);
            std::cout << "[" << nom << "] " << avion->getNom() 
                      << " en CROISIERE (" << (int)avion->getAltitude() << "m, "
                      << (int)avion->getVitesse() << "m/s)" << std::endl;
        }
    }
    
    gererSeparationVerticale();
    detecterConflits();
}

void CCR::ajouterSecteur(const std::string& nom_secteur, const ZoneControle& zone_secteur, int capacite) {
    Secteur s;
    s.nom = nom_secteur;
    s.zone = zone_secteur;
    s.capacite_max = capacite;
    s.nb_avions_actuels = 0;
    secteurs.push_back(s);
}

CCR::Secteur* CCR::trouverSecteur(const Position& pos) {
    for (auto& secteur : secteurs) {
        if (secteur.zone.contient(pos)) {
            return &secteur;
        }
    }
    return nullptr;
}

void CCR::gererSeparationVerticale() {
    std::lock_guard<std::mutex> lock(mutex_avions);
    const double SEPARATION_VERTICALE = 1000;
    
    for (auto& avion : avions_surveilles) {
        if (avion->getEtat() == EtatAvion::CROISIERE) {
            double alt = avion->getAltitude();
            double niveau_vol = std::round(alt / SEPARATION_VERTICALE) * SEPARATION_VERTICALE;
            if (std::abs(alt - niveau_vol) > 50) {
                std::lock_guard<std::mutex> console_lock(console_mutex);
                std::cout << "[" << nom << "] ⚠️  " << avion->getNom() 
                          << " hors niveau de vol" << std::endl;
            }
        }
    }
}

bool CCR::doitTransfererVersAPP(const Avion* avion, double distance_destination) const {
    return distance_destination < 50000 && avion->getEtat() == EtatAvion::CROISIERE;
}