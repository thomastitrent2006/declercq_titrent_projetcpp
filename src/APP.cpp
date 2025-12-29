#include "../include/APP.h"
#include "../include/TWR.h"
#include <iostream>
#include <cmath>
#include <iomanip>

APP::APP(const std::string& nom, const Position& centre, double rayon)
    : ControleurBase(nom), centreAeroport(centre), rayonControle(rayon),
    towerReference(nullptr) {
}

void APP::processLogic() {
    gererNouvelArrivant(nullptr); // Vérifier les nouveaux avions
    gererUrgences();
    gererTrajectoires();
}

void APP::gererNouvelArrivant(Avion* avion) {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto* a : avionsSousControle) {
        if (a->getEtat() == EtatAvion::EN_ROUTE) {
            // L'avion entre dans la zone d'approche
            Position pos = a->getPosition();
            double distance = pos.distanceTo(centreAeroport);

            if (distance <= rayonControle) {
                a->setEtat(EtatAvion::APPROCHE);
                fileAttenteAtterrissage.push(a->getNom());

                // Assigner une trajectoire circulaire
                int niveau = static_cast<int>(fileAttenteAtterrissage.size());
                assignerTrajectoireCirculaire(a, niveau);

                logAction("ENTREE_ZONE_APPROCHE",
                    "Avion " + a->getNom() + " entre en zone d'approche, niveau " +
                    std::to_string(niveau));
            }
        }
    }
}

void APP::gererUrgences() {
    std::lock_guard<std::mutex> lock(mtx);

    for (auto* avion : avionsSousControle) {
        if (avion->estEnUrgence() && avion->getEtat() == EtatAvion::APPROCHE) {
            // Priorité absolue pour les urgences
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

void APP::gererTrajectoires() {
    std::lock_guard<std::mutex> lock(mtx);

    if (!fileAttenteAtterrissage.empty() && towerReference != nullptr) {
        std::string avionId = fileAttenteAtterrissage.front();

        // Vérifier si on peut demander l'autorisation d'atterrissage
        if (demanderAutorisationAtterrissage(avionId)) {
            fileAttenteAtterrissage.pop();

            // Trouver l'avion et changer son état
            for (auto* avion : avionsSousControle) {
                if (avion->getNom() == avionId) {
                    avion->setEtat(EtatAvion::ATTENTE_ATTERRISSAGE);

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
    // Trajectoire circulaire à différents niveaux d'altitude
    double rayonTrajectoire = rayonControle * 0.8 - (niveau * 1000.0);
    double altitude = 1000.0 + (niveau * 500.0); // Chaque niveau à 500m d'écart

    logAction("TRAJECTOIRE_ASSIGNEE",
        "Avion " + avion->getNom() + " - Trajectoire circulaire rayon " +
        std::to_string(static_cast<int>(rayonTrajectoire)) + "m, altitude " +
        std::to_string(static_cast<int>(altitude)) + "m");
}

void APP::afficherConsole() const {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║  CONTRÔLE D'APPROCHE - " << std::left << std::setw(12) << nom << "   ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║ Zone de contrôle: " << std::setw(19) << (static_cast<int>(rayonControle / 1000)) << " km║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║ Avions sous contrôle: " << std::setw(16) << avionsSousControle.size() << "║\n";
    std::cout << "╠════════════════════════════════════════╣\n";

    if (avionsSousControle.empty()) {
        std::cout << "║ Aucun avion en approche                ║\n";
    }
    else {
        std::cout << "║ ID      État         Distance  Alt.    ║\n";
        std::cout << "╠════════════════════════════════════════╣\n";

        for (const auto* avion : avionsSousControle) {
            Position pos = avion->getPosition();
            double distance = pos.distanceTo(centreAeroport) / 1000.0; // en km

            std::string etatStr;
            switch (avion->getEtat()) {
            case EtatAvion::EN_ROUTE: etatStr = "EN_ROUTE"; break;
            case EtatAvion::APPROCHE: etatStr = "APPROCHE"; break;
            case EtatAvion::URGENCE: etatStr = "URGENCE!"; break;
            case EtatAvion::ATTENTE_ATTERRISSAGE: etatStr = "ATTENTE"; break;
            default: etatStr = "AUTRE"; break;
            }

            std::cout << "║ " << std::left << std::setw(8) << avion->getNom()
                << std::setw(13) << etatStr
                << std::right << std::setw(6) << std::fixed << std::setprecision(1) << distance << "km"
                << std::setw(7) << static_cast<int>(pos.altitude) << "m ║\n";
        }
    }

    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║ File d'attente: " << std::setw(22) << fileAttenteAtterrissage.size() << "║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
}