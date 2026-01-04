#include "../include/TWR.h"
#include <iostream>
#include <iomanip>

TWR::TWR(const std::string& nom) : ControleurBase(nom) {
    piste.occupee = false;
    piste.avionActuel = "";
    initialiserParkings(10);
}

void TWR::initialiserParkings(int nombre) {
    std::lock_guard<std::mutex> lock(mtx);

    for (int i = 1; i <= nombre; i++) {
        Parking p;
        p.id = "P" + std::to_string(i);
        p.occupee = false;
        p.avionActuel = "";
        p.distancePiste = 100.0 * i;
        p.position = Position(50.0 * i, 100.0, 0);

        parkings[p.id] = p;
    }

    logAction("INIT_PARKINGS", "Initialisation de " + std::to_string(nombre) + " parkings");
}

bool TWR::pisteLibre() const {
    std::lock_guard<std::mutex> lock(mtx);
    return pisteLibreInternal();
}

bool TWR::pisteLibreInternal() const {
    if (piste.occupee) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - piste.heureLiberation).count();
        return elapsed >= 0;
    }
    return true;
}

bool TWR::autoriserAtterrissage(const std::string& avionId) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!pisteLibreInternal()) {
        return false;
    }

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

    return "";
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

    if (piste.occupee) {
        auto now = std::chrono::steady_clock::now();
        if (now >= piste.heureLiberation) {
            std::string parkingId = assignerParking();

            if (!parkingId.empty()) {
                for (auto* avion : avionsSousControle) {
                    if (avion->getNom() == piste.avionActuel) {
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
    std::lock_guard<std::mutex> lock(mtx);

    for (auto* avion : avionsSousControle) {
        if (avion->getEtat() == EtatAvion::ROULAGE_ARRIVEE) {
            avion->setEtat(EtatAvion::PARKING);
            logAction("AVION_STATIONNE", "Avion " + avion->getNom() + " stationné");
        }
    }
}

void TWR::gererDecollages() {
    std::lock_guard<std::mutex> lock(mtx);

    if (!piste.occupee && !avionsSousControle.empty()) {
        Avion* avionPrioritaire = nullptr;
        double distanceMax = 0;

        for (auto* avion : avionsSousControle) {
            if (avion->getEtat() == EtatAvion::PARKING) {
                for (const auto& pair : parkings) {
                    if (pair.second.occupee && pair.second.avionActuel == avion->getNom()) {
                        if (pair.second.distancePiste > distanceMax) {
                            distanceMax = pair.second.distancePiste;
                            avionPrioritaire = avion;
                        }
                        break;
                    }
                }
            }
        }

        if (avionPrioritaire != nullptr) {
            avionPrioritaire->setEtat(EtatAvion::ROULAGE_DECOLLAGE);

            // Libérer le parking directement (on a déjà le lock)
            for (auto& pair : parkings) {
                if (pair.second.occupee && pair.second.avionActuel == avionPrioritaire->getNom()) {
                    pair.second.occupee = false;
                    pair.second.avionActuel = "";
                    logAction("LIBERATION_PARKING", "Parking " + pair.first + " libéré");
                    break;
                }
            }

            logAction("AUTORISATION_DECOLLAGE",
                "Avion " + avionPrioritaire->getNom() + " autorisé à rouler vers la piste");
        }
    }
}

std::string TWR::assignerParking() {
    for (auto& pair : parkings) {
        if (!pair.second.occupee) {
            return pair.first;
        }
    }
    return "";
}

void TWR::afficherPlanAeroport() const {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "\n=== TOUR DE CONTROLE - " << nom << " ===\n";
    std::cout << "PISTE: " << (piste.occupee ? "OCCUPEE [" + piste.avionActuel + "]" : "LIBRE") << "\n";
    std::cout << "\nPARKINGS:\n";

    for (const auto& pair : parkings) {
        std::cout << "  " << pair.first << ": ";
        if (pair.second.occupee) {
            std::cout << pair.second.avionActuel;
        }
        else {
            std::cout << "DISPONIBLE";
        }
        std::cout << "\n";
    }

    std::cout << "=============================\n";
}