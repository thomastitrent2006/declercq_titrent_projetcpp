#include "../include/CCR.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

CCR::CCR(const std::string& nom, double altitude)
    : ControleurBase(nom), altitudeCroisiere(altitude) {
}

void CCR::ajouterAeroport(const std::string& nom, const Position& pos,
    APP* app, int capacite) {
    std::lock_guard<std::mutex> lock(mtx);

    Aeroport aeroport;
    aeroport.nom = nom;
    aeroport.position = pos;
    aeroport.controleurApproche = app;
    aeroport.capaciteMax = capacite;
    aeroport.avionsEnApproche = 0;

    aeroports[nom] = aeroport;

    logAction("AJOUT_AEROPORT", "Aéroport " + nom + " ajouté au réseau");
}

void CCR::ajouterRoute(const std::string& depart, const std::string& arrivee) {
    std::lock_guard<std::mutex> lock(mtx);

    auto itDepart = aeroports.find(depart);
    auto itArrivee = aeroports.find(arrivee);

    if (itDepart == aeroports.end() || itArrivee == aeroports.end()) {
        logAction("ERREUR_ROUTE", "Aéroport inexistant pour la route " +
            depart + " -> " + arrivee);
        return;
    }

    Route route;
    route.depart = depart;
    route.arrivee = arrivee;
    route.distance = itDepart->second.position.distanceTo(itArrivee->second.position);

    // Créer des waypoints simples (début, milieu, fin)
    route.waypoints.push_back(itDepart->second.position);

    Position milieu(
        (itDepart->second.position.x + itArrivee->second.position.x) / 2.0,
        (itDepart->second.position.y + itArrivee->second.position.y) / 2.0,
        altitudeCroisiere
    );
    route.waypoints.push_back(milieu);
    route.waypoints.push_back(itArrivee->second.position);

    routes.push_back(route);

    logAction("AJOUT_ROUTE", "Route " + depart + " -> " + arrivee +
        " ajoutée (" + std::to_string(static_cast<int>(route.distance / 1000)) + " km)");
}

void CCR::creerVol(const std::string& nomAvion, const std::string& depart,
    const std::string& arrivee) {
    std::lock_guard<std::mutex> lock(mtx);

    auto itDepart = aeroports.find(depart);
    auto itArrivee = aeroports.find(arrivee);

    if (itDepart == aeroports.end() || itArrivee == aeroports.end()) {
        logAction("ERREUR_VOL", "Impossible de créer le vol " + nomAvion +
            " - aéroport inexistant");
        return;
    }

    // Vérifier la capacité de l'aéroport d'arrivée
    if (!verifierCapaciteAeroport(arrivee)) {
        logAction("VOL_RETARDE", "Vol " + nomAvion + " retardé - capacité " +
            arrivee + " saturée");
        return;
    }

    // Créer l'avion
    Position posDepart = itDepart->second.position;
    posDepart.altitude = altitudeCroisiere;

    Position posArrivee = itArrivee->second.position;
    posArrivee.altitude = altitudeCroisiere;

    Avion* avion = new Avion(nomAvion, posDepart, posArrivee);
    avion->setEtat(EtatAvion::EN_ROUTE);
    avion->setVitesse(250.0); // 250 m/s ≈ 900 km/h

    ajouterAvion(avion);

    // Incrémenter le compteur de l'aéroport de destination
    aeroports[arrivee].avionsEnApproche++;

    logAction("VOL_CREE", "Vol " + nomAvion + " créé: " + depart + " -> " + arrivee);
}

void CCR::processLogic() {
    gererSeparation();
    gererFlux();
    transfererVersAPP();
}

void CCR::gererSeparation() {
    std::lock_guard<std::mutex> lock(mtx);

    const double SEPARATION_MINIMALE = 5000.0; // 5 km
    const double SEPARATION_VERTICALE = 300.0;  // 300 m

    // Vérifier toutes les paires d'avions
    for (size_t i = 0; i < avionsSousControle.size(); i++) {
        for (size_t j = i + 1; j < avionsSousControle.size(); j++) {
            Avion* a1 = avionsSousControle[i];
            Avion* a2 = avionsSousControle[j];

            Position pos1 = a1->getPosition();
            Position pos2 = a2->getPosition();

            double distanceHorizontale = pos1.distanceTo(pos2);
            double distanceVerticale = std::abs(pos1.altitude - pos2.altitude);

            // Conflit détecté
            if (distanceHorizontale < SEPARATION_MINIMALE &&
                distanceVerticale < SEPARATION_VERTICALE) {

                logAction("CONFLIT_DETECTE",
                    "Conflit entre " + a1->getNom() + " et " + a2->getNom() +
                    " - distance: " + std::to_string(static_cast<int>(distanceHorizontale)) + "m");

                // Résolution simple: modifier l'altitude d'un des avions
                Position nouvellePos = pos2;
                nouvellePos.altitude += 500.0;
                a2->setPosition(nouvellePos);

                logAction("RESOLUTION_CONFLIT",
                    a2->getNom() + " monte à " +
                    std::to_string(static_cast<int>(nouvellePos.altitude)) + "m");
            }
        }
    }
}

void CCR::gererFlux() {
    std::lock_guard<std::mutex> lock(mtx);

    // Vérifier que les aéroports ne sont pas surchargés
    for (auto& pair : aeroports) {
        Aeroport& aeroport = pair.second;

        if (aeroport.avionsEnApproche >= aeroport.capaciteMax) {
            logAction("AEROPORT_SATURE",
                "Aéroport " + aeroport.nom + " à capacité maximale (" +
                std::to_string(aeroport.avionsEnApproche) + "/" +
                std::to_string(aeroport.capaciteMax) + ")");
        }
    }
}

void CCR::transfererVersAPP() {
    std::lock_guard<std::mutex> lock(mtx);

    std::vector<Avion*> avionsARetirer;

    for (auto* avion : avionsSousControle) {
        Position pos = avion->getPosition();
        Position dest = avion->getDestination();

        // Chercher l'aéroport de destination
        for (auto& pair : aeroports) {
            Aeroport& aeroport = pair.second;

            double distanceDestination = dest.distanceTo(aeroport.position);

            // L'avion est proche de sa destination
            if (distanceDestination < 1000.0) {
                double distanceActuelle = pos.distanceTo(aeroport.position);

                // L'avion entre dans la zone d'approche (50 km)
                if (distanceActuelle < 50000.0 && aeroport.controleurApproche != nullptr) {

                    logAction("TRANSFERT_APP",
                        "Avion " + avion->getNom() + " transféré à l'APP " +
                        aeroport.nom);

                    // Transférer l'avion à l'APP
                    aeroport.controleurApproche->ajouterAvion(avion);

                    // Message de transfert
                    Message msg;
                    msg.expediteur = nom;
                    msg.destinataire = aeroport.controleurApproche->getNom();
                    msg.type = "TRANSFERT_AVION";
                    msg.avionId = avion->getNom();
                    msg.contenu = "Avion transféré pour approche";
                    msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count();

                    envoyerMessage(msg);

                    avionsARetirer.push_back(avion);

                    // Décrémenter le compteur
                    if (aeroport.avionsEnApproche > 0) {
                        aeroport.avionsEnApproche--;
                    }

                    break;
                }
            }
        }
    }

    // Retirer les avions transférés
    for (auto* avion : avionsARetirer) {
        retirerAvion(avion->getNom());
    }
}

bool CCR::verifierCapaciteAeroport(const std::string& aeroportId) {
    auto it = aeroports.find(aeroportId);
    if (it == aeroports.end()) {
        return false;
    }

    return it->second.avionsEnApproche < it->second.capaciteMax;
}

double CCR::calculerSeparationMinimale(Avion* a1, Avion* a2) {
    Position pos1 = a1->getPosition();
    Position pos2 = a2->getPosition();

    return pos1.distance3DTo(pos2);
}

std::vector<std::pair<std::string, std::string>> CCR::detecterRisquesCollision() const {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::pair<std::string, std::string>> risques;

    const double DISTANCE_ALERTE = 10000.0; // 10 km

    for (size_t i = 0; i < avionsSousControle.size(); i++) {
        for (size_t j = i + 1; j < avionsSousControle.size(); j++) {
            double distance = calculerSeparationMinimale(
                avionsSousControle[i], avionsSousControle[j]);

            if (distance < DISTANCE_ALERTE) {
                risques.push_back({
                    avionsSousControle[i]->getNom(),
                    avionsSousControle[j]->getNom()
                    });
            }
        }
    }

    return risques;
}

void CCR::afficherEspaceAerien() const {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "\n╔═══════════════════════════════════════════════════════╗\n";
    std::cout << "║     CENTRE DE CONTRÔLE RÉGIONAL - " << std::left << std::setw(16) << nom << "   ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════╣\n";
    std::cout << "║ Altitude de croisière: " << std::right << std::setw(23)
        << static_cast<int>(altitudeCroisiere) << "m    ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════╣\n";

    // Afficher les aéroports
    std::cout << "║ AÉROPORTS:                                            ║\n";
    for (const auto& pair : aeroports) {
        const Aeroport& ap = pair.second;
        std::cout << "║  • " << std::left << std::setw(15) << ap.nom
            << " [" << ap.avionsEnApproche << "/" << ap.capaciteMax << "]"
            << std::string(31 - ap.nom.length(), ' ') << "║\n";
    }

    std::cout << "╠═══════════════════════════════════════════════════════╣\n";
    std::cout << "║ VOLS EN ROUTE: " << std::setw(38) << avionsSousControle.size() << "║\n";

    if (!avionsSousControle.empty()) {
        std::cout << "╠═══════════════════════════════════════════════════════╣\n";
        std::cout << "║ ID        Altitude    Vitesse    Carburant   État    ║\n";
        std::cout << "╠═══════════════════════════════════════════════════════╣\n";

        for (const auto* avion : avionsSousControle) {
            Position pos = avion->getPosition();
            std::cout << "║ " << std::left << std::setw(10) << avion->getNom()
                << std::right << std::setw(7) << static_cast<int>(pos.altitude) << "m"
                << std::setw(8) << static_cast<int>(avion->getVitesse()) << "m/s"
                << std::setw(9) << static_cast<int>(avion->getCarburant()) << "L"
                << std::setw(10) << (avion->estEnUrgence() ? "URGENCE!" : "OK")
                << "  ║\n";
        }
    }

    // Détecter les risques de collision
    auto risques = detecterRisquesCollision();
    if (!risques.empty()) {
        std::cout << "╠═══════════════════════════════════════════════════════╣\n";
        std::cout << "║ ⚠️  ALERTES PROXIMITÉ: " << std::setw(30) << risques.size() << "║\n";
        for (const auto& paire : risques) {
            std::cout << "║    " << std::left << std::setw(10) << paire.first
                << " ↔ " << std::setw(36) << paire.second << "║\n";
        }
    }

    std::cout << "╚═══════════════════════════════════════════════════════╝\n";
}