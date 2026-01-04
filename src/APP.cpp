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
    gererNouvellesArrivees();
    /*gererUrgences();*/
    gererTrajectoires();
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

    std::lock_guard<std::mutex> lock(mtx);

    // Vérifier si l'avion n'est pas déjà dans la liste
    for (auto* a : avionsEnApproche) {
        if (a->getNom() == avion->getNom()) {
            return;
        }
    }

    avionsEnApproche.push_back(avion);
    ajouterAvion(avion); // Appel à la méthode de ControleurBase

    logAction("AVION_AJOUTE", "Avion " + avion->getNom() + " ajouté en approche");
}

void APP::retirerAvionEnApproche(Avion* avion) {
    if (avion == nullptr) return;

    std::lock_guard<std::mutex> lock(mtx);

    for (size_t i = 0; i < avionsEnApproche.size(); i++) {
        if (avionsEnApproche[i]->getNom() == avion->getNom()) {
            avionsEnApproche.erase(avionsEnApproche.begin() + i);
            retirerAvion(avion->getNom()); // Appel à la méthode de ControleurBase
            logAction("AVION_RETIRE", "Avion " + avion->getNom() + " retiré de l'approche");
            return;
        }
    }
}

void APP::gererNouvellesArrivees() {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto* avion : avionsSousControle) {
        // Les états possibles avant l'approche sont CROISIERE ou DESCENTE
        if (avion->getEtat() == EtatAvion::CROISIERE ||
            avion->getEtat() == EtatAvion::DESCENTE) {
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

/* void APP::gererUrgences() {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto* avion : avionsSousControle) {
        if (avion->estEnUrgence() && avion->getEtat() == EtatAvion::APPROCHE) {
            logAction("URGENCE_DETECTEE",
                "Avion " + avion->getNom() + " signale une urgence - carburant faible");

            // Placer en tête de file
            std::queue<std::string> nouvelleFile;
            nouvelleFile.push(avion->getNom());

            while (!fileAttenteAtterrissage.empty()) {
                std::string id = fileAttenteAtterrissage.front();
                fileAttenteAtterrissage.pop();
                if (id != avion->getNom()) {
                    nouvelleFile.push(id);
                }
            }

            fileAttenteAtterrissage = nouvelleFile;
            avion->setEtat(EtatAvion::URGENCE);
        }
    }
}
*/
void APP::gererTrajectoires() {
    std::lock_guard<std::mutex> lock(mtx);

    if (!fileAttenteAtterrissage.empty() && towerReference != nullptr) {
        std::string avionId = fileAttenteAtterrissage.front();

        if (demanderAutorisationAtterrissage(avionId)) {
            fileAttenteAtterrissage.pop();

            for (auto* avion : avionsSousControle) {
                if (avion->getNom() == avionId) {
                    // Changer l'état vers ATTERRISSAGE plutôt que ATTENTE_ATTERRISSAGE
                    avion->setEtat(EtatAvion::ATTERRISSAGE);

                    Message msg;
                    msg.expediteur = nom;
                    msg.destinataire = "TWR";
                    msg.type = "TRANSFERT_AVION";
                    msg.avionId = avionId;
                    msg.contenu = "Avion transféré pour atterrissage";
                    msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                    envoyerMessage(msg);

                    logAction("TRANSFERT_TWR", "Avion " + avionId + " transféré à la tour");
                    break;
                }
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

APP* APP::demanderNouvelAPP() {
    // Si le CCR est disponible, lui demander un nouvel APP
    if (ccrReference != nullptr) {
        logAction("DEMANDE_NOUVEL_APP",
            "Zone saturée, demande d'un nouveau contrôleur d'approche");
        // Le CCR devrait créer un nouvel APP
        return nullptr; // À implémenter dans CCR
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
