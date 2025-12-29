#include "../include/TWR.h"
#include <iostream>
#include <iomanip>

TWR::TWR(const std::string& nom) : ControleurBase(nom) {
    piste.occupee = false;
    piste.avionActuel = "";
    initialiserParkings(10); // 10 parkings par défaut
}

void TWR::initialiserParkings(int nombre) {
    std::lock_guard<std::mutex> lock(mtx);

    for (int i = 1; i <= nombre; i++) {
        Parking p;
        p.id = "P" + std::to_string(i);
        p.occupee = false;
        p.avionActuel = "";
        p.distancePiste = 100.0 * i; // Plus le numéro est élevé, plus c'est loin
        p.position = Position(50.0 * i, 100.0, 0);

        parkings[p.id] = p;
    }

    logAction("INIT_PARKINGS", "Initialisation de " + std::to_string(nombre) + " parkings");
}

bool TWR::pisteLibre() const {
    std::lock_guard<std::mutex> lock(mtx);

    if (piste.occupee) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - piste.heureLiberation).count();

        // La piste sera bientôt libre
        return elapsed >= 0;
    }

    return true;
}

bool TWR::autoriserAtterrissage(const std::string& avionId) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!pisteLibre()) {
        return false;
    }

    // Occuper la piste
    piste.occupee = true;
    piste.avionActuel = avionId;
    piste.heureLiberation = std::chrono::steady_clock::now() +
        std::chrono::seconds(piste.DUREE_ATTERRISSAGE);

    logAction("AUTORISATION_ATTERRISSAGE", "Avion " + avionId + " autorisé à atterrir");
    return true;
}

std::string TWR::getParkingDisponible() const {
    std::lock_guard<std::mutex> lock(mtx);

    for (const auto& pair : parkings) {
        if (!pair.second.occupee) {
            return pair.first;
        }
    }

    return ""; // Aucun parking disponible
}

void TWR::libererParking(const std::string& parkingId) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = parkings.find(parkingId);
    if (it != parkings.end()) {
        it->second.occupee = false;
        it->second.avionActuel = "";
        logAction("LIBERATION_PARKING", "Parking " + parkingId + " libéré");
    }
}

void TWR::processLogic() {
    gererAtterrissages();
    gererRoulage();
    gererDecollages();
}

void TWR::gererAtterrissages() {
    std::lock_guard<std::mutex> lock(mtx);

    // Vérifier si la piste doit être libérée
    if (piste.occupee) {
        auto now = std::chrono::steady_clock::now();
        if (now >= piste.heureLiberation) {
            // Avion a atterri, assigner un parking
            std::string parkingId = assignerParking();

            if (!parkingId.empty()) {
                // Trouver l'avion et l'assigner au parking
                for (auto* avion : avionsSousControle) {
                    if (avion->getNom() == piste.avionActuel) {
                        avion->setParkingAssigne(parkingId);
                        avion->setEtat(EtatAvion::ROULAGE_ARRIVEE);

                        parkings[parkingId].occupee = true;
                        parkings[parkingId].avionActuel = piste.avionActuel;

                        logAction("ROULAGE_VERS_PARKING",
                            "Avion " + piste.avionActuel + " roule vers " + parkingId);
                        break;
                    }
                }
            }

            piste.occupee = false;
            piste.avionActuel = "";
        }
    }
}

void TWR::gererRoulage() {
    // Simuler le roulage des avions
    std::lock_guard<std::mutex> lock(mtx);

    for (auto* avion : avionsSousControle) {
        if (avion->getEtat() == EtatAvion::ROULAGE_ARRIVEE) {
            // Simulation: l'avion arrive au parking après un certain temps
            // Dans une vraie simulation, cela dépendrait de la distance
            avion->setEtat(EtatAvion::STATIONNE);
            logAction("AVION_STATIONNE", "Avion " + avion->getNom() +
                " stationné au parking " + avion->getParkingAssigne());
        }
    }
}

void TWR::gererDecollages() {
    std::lock_guard<std::mutex> lock(mtx);

    // Logique de priorité: l'avion le plus éloigné de la piste part en premier
    if (!piste.occupee && !avionsSousControle.empty()) {
        Avion* avionPrioritaire = nullptr;
        double distanceMax = 0;

        for (auto* avion : avionsSousControle) {
            if (avion->getEtat() == EtatAvion::STATIONNE) {
                std::string parking = avion->getParkingAssigne();
                if (!parking.empty() && parkings[parking].distancePiste > distanceMax) {
                    distanceMax = parkings[parking].distancePiste;
                    avionPrioritaire = avion;
                }
            }
        }

        if (avionPrioritaire != nullptr) {
            avionPrioritaire->setEtat(EtatAvion::ROULAGE_DEPART);
            libererParking(avionPrioritaire->getParkingAssigne());
            logAction("AUTORISATION_DECOLLAGE",
                "Avion " + avionPrioritaire->getNom() + " autorisé à rouler vers la piste");
        }
    }
}

std::string TWR::assignerParking() {
    // Assigner le premier parking disponible
    for (auto& pair : parkings) {
        if (!pair.second.occupee) {
            return pair.first;
        }
    }
    return "";
}

void TWR::afficherPlanAeroport() const {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║     TOUR DE CONTRÔLE - " << std::left << std::setw(12) << nom << "   ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║ PISTE: " << (piste.occupee ? "OCCUPÉE [" + piste.avionActuel + "]" : "LIBRE")
        << std::string(26 - (piste.occupee ? piste.avionActuel.length() + 10 : 5), ' ') << "║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║ PARKINGS:                              ║\n";

    for (const auto& pair : parkings) {
        std::cout << "║  " << std::left << std::setw(4) << pair.first << ": ";
        if (pair.second.occupee) {
            std::cout << std::setw(30) << pair.second.avionActuel;
        }
        else {
            std::cout << std::setw(30) << "DISPONIBLE";
        }
        std::cout << " ║\n";
    }

    std::cout << "╚════════════════════════════════════════╝\n";
}