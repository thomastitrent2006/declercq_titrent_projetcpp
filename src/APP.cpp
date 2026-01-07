#include "../include/APP.h"
#include "../include/TWR.h"
#include "../include/CCR.h"
#include <iostream>
#include <cmath>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

APP::APP(const std::string& nom, const Position& centre, float rayon,
    TWR* twr, CCR* ccr)
    : ControleurBase(nom),
    centreAeroport(centre),
    rayonControle(rayon),
    towerReference(twr),
    ccrReference(ccr) {
}

void APP::processLogic() {
    std::lock_guard<std::mutex> lock(mtx);

    static int compteur = 0;
    if (compteur++ % 50 == 0) {
        if (!avionsSousControle.empty()) {
            std::cout << "[APP " << nom << "] " << avionsSousControle.size() << " avions\n";
            for (auto* avion : avionsSousControle) {
                std::cout << "  - " << avion->getNom()
                    << " | " << avion->getEtatString() << "\n";
            }
        }
    }

    gererNouvellesArrivees();
    gererTrajectoires();
    gererDeparts();
}

bool APP::estDansZone(const Avion& avion) const {
    return estDansZone(avion.getPosition());
}

bool APP::estDansZone(const Position& pos) const {
    double distance = pos.distanceTo(centreAeroport);
    return distance <= rayonControle;
}

void APP::ajouterAvionEnApproche(Avion* avion) {
    if (avion == nullptr) return;

    // Vérifier si l'avion n'est pas déjà dans la liste
    for (auto* a : avionsEnApproche) {
        if (a->getNom() == avion->getNom()) {
            return;
        }
    }

    avionsEnApproche.push_back(avion);
    ajouterAvion(avion);

    logAction("AVION_AJOUTE", "Avion " + avion->getNom() + " ajouté en approche");
}

void APP::retirerAvionEnApproche(Avion* avion) {
    if (avion == nullptr) return;

    for (size_t i = 0; i < avionsEnApproche.size(); i++) {
        if (avionsEnApproche[i]->getNom() == avion->getNom()) {
            avionsEnApproche.erase(avionsEnApproche.begin() + i);
            retirerAvion(avion->getNom());
            logAction("AVION_RETIRE", "Avion " + avion->getNom() + " retiré de l'approche");
            return;
        }
    }
}

void APP::gererNouvellesArrivees() {
    for (auto* avion : avionsSousControle) {
   
        if (avion->getEtat() == EtatAvion::DESCENTE) {  
            Position pos = avion->getPosition();

            if (estDansZone(pos)) {
                avion->setEtat(EtatAvion::APPROCHE);

                // Vérifier si l'avion n'est pas déjà dans la file
                bool dejaDansFile = false;
                std::queue<std::string> tempQueue = fileAttenteAtterrissage;
                while (!tempQueue.empty()) {
                    if (tempQueue.front() == avion->getNom()) {
                        dejaDansFile = true;
                        break;
                    }
                    tempQueue.pop();
                }

                if (!dejaDansFile) {
                    fileAttenteAtterrissage.push(avion->getNom());
                }

                int niveau = static_cast<int>(fileAttenteAtterrissage.size());
                assignerTrajectoireCirculaire(avion, niveau);

                logAction("ENTREE_ZONE_APPROCHE",
                    "Avion " + avion->getNom() + " entre en zone d'approche, niveau " +
                    std::to_string(niveau));
            }
        }
    }
}

void APP::gererTrajectoires() {
    for (auto* avion : avionsSousControle) {
        if (avion == nullptr) continue;

        EtatAvion etat = avion->getEtat();

        if (etat == EtatAvion::ATTERRISSAGE) {
            bool pisteOccupee = false;

            if (towerReference != nullptr) {
                pisteOccupee = towerReference->isPisteOccupee();  // ← Utilise piste.occupee
            }

            if (pisteOccupee) {
                avion->setEtat(EtatAvion::ATTENTE);
                avion->setCentreAttente(centreAeroport);
                std::cout << "[" << avion->getNom() << "] Piste occupée - Mise en attente\n";
            }
        }

        if (etat == EtatAvion::ATTENTE) {
            bool pisteOccupee = false;

            if (towerReference != nullptr) {
                pisteOccupee = towerReference->isPisteOccupee();
            }

            if (!pisteOccupee) {
                avion->setEtat(EtatAvion::ATTERRISSAGE);
                std::cout << "[" << avion->getNom() << "] Piste libre - Autorisation d'atterrir\n";
            }
        }
    }
}
bool APP::demanderAutorisationAtterrissage(const std::string& avionId) {
    if (towerReference == nullptr) {
        return false;
    }

    if (towerReference->pisteLibre()) {
        return towerReference->autoriserAtterrissage(avionId);
    }

    return false;
}

void APP::assignerTrajectoireCirculaire(Avion* avion, int niveau) {
    double rayonTrajectoire = rayonControle * 0.8 - (niveau * 1000.0);
    double altitude = 1000.0 + (niveau * 500.0);

    logAction("TRAJECTOIRE_ASSIGNEE",
        "Avion " + avion->getNom() + " - Trajectoire circulaire rayon " +
        std::to_string(static_cast<int>(rayonTrajectoire)) + "m, altitude " +
        std::to_string(static_cast<int>(altitude)) + "m");
}

void APP::gererDeparts() {
    std::vector<Avion*> avionsARetirer;

    for (auto* avion : avionsSousControle) {
        if (avion == nullptr) continue;

        EtatAvion etat = avion->getEtat();
        Position pos = avion->getPosition();

        // Si l'avion est en CROISIERE et loin (> 55 km)
        if (etat == EtatAvion::CROISIERE) {
            double distance = pos.distanceTo(centreAeroport);
            if (distance > 55000.0) {
                avionsARetirer.push_back(avion);
            }
        }
    }

    // Retirer les avions de l'APP ET les redonner au CCR
    for (auto* avion : avionsARetirer) {
        auto it = std::find(avionsSousControle.begin(), avionsSousControle.end(), avion);
        if (it != avionsSousControle.end()) {
            avionsSousControle.erase(it);

            // ✅ REDONNER L'AVION AU CCR
            if (ccrReference != nullptr) {
                ccrReference->ajouterAvion(avion);
            }
        }
    }
}

APP* APP::demanderNouvelAPP() {
    if (ccrReference != nullptr) {
        logAction("DEMANDE_NOUVEL_APP",
            "Zone saturée, demande d'un nouveau contrôleur d'approche");
        return nullptr;
    }
    return nullptr;
}

void APP::afficherConsole() const {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "\n=== CONTROLE D'APPROCHE - " << nom << " ===\n";
    std::cout << "Zone de controle: " << static_cast<int>(rayonControle / 1000.0) << " km\n";
    std::cout << "Centre: (" << centreAeroport.x << ", " << centreAeroport.y << ")\n";
    std::cout << "Avions sous controle: " << avionsSousControle.size() << "\n";
    std::cout << "File d'attente: " << fileAttenteAtterrissage.size() << "\n";

    if (!avionsSousControle.empty()) {
        std::cout << "\n--- AVIONS EN APPROCHE ---\n";
        for (const auto* avion : avionsSousControle) {
            Position pos = avion->getPosition();
            double distance = pos.distanceTo(centreAeroport) / 1000.0;

            std::string etatStr;
            switch (avion->getEtat()) {
            case EtatAvion::PARKING: etatStr = "PARKING"; break;
            case EtatAvion::ROULAGE_DECOLLAGE: etatStr = "ROULAGE_DEC"; break;
            case EtatAvion::DECOLLAGE: etatStr = "DECOLLAGE"; break;
            case EtatAvion::MONTEE: etatStr = "MONTEE"; break;
            case EtatAvion::CROISIERE: etatStr = "CROISIERE"; break;
            case EtatAvion::DESCENTE: etatStr = "DESCENTE"; break;
            case EtatAvion::APPROCHE: etatStr = "APPROCHE"; break;
            case EtatAvion::ATTERRISSAGE: etatStr = "ATTERRISSAGE"; break;
            case EtatAvion::ROULAGE_ARRIVEE: etatStr = "ROULAGE_ARR"; break;
            default: etatStr = "AUTRE"; break;
            }

            std::cout << "  " << avion->getNom()
                << " | " << etatStr
                << " | " << std::fixed << std::setprecision(2) << distance << " km"
                << " | Alt: " << static_cast<int>(pos.altitude) << "m\n";
        }
    }
    std::cout << "=====================================\n";
}

void APP::transfererAvionVersCCR(Avion* avion) {
    if (avion == nullptr || ccrReference == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(mtx);

    // Vérifier que l'avion est bien en dehors de notre zone
    if (!estDansZone(avion->getPosition())) {
        logAction("TRANSFERT_CCR",
            "Avion " + avion->getNom() + " transféré au CCR");

        // Notifier le CCR
        ccrReference->recevoirAvionDepuisAPP(avion, nom);

        // Retirer l'avion de notre contrôle
        retirerAvion(avion->getNom());
    }
}