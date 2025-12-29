#ifndef TWR_H
#define TWR_H

#include "ControleurBase.h"
#include <map>
#include <chrono>
#include <queue>

struct Piste {
    bool occupee;
    std::string avionActuel;
    std::chrono::steady_clock::time_point heureLiberation;
    const int DUREE_ATTERRISSAGE = 30; // secondes
    const int DUREE_DECOLLAGE = 20;    // secondes
};

struct Parking {
    std::string id;
    bool occupee;
    std::string avionActuel;
    double distancePiste; // en mètres
    Position position;
};

class TWR : public ControleurBase {
private:
    Piste piste;
    std::map<std::string, Parking> parkings;
    std::queue<std::string> fileDecollage;

    void processLogic() override;
    void gererAtterrissages();
    void gererDecollages();
    void gererRoulage();
    std::string assignerParking();

public:
    TWR(const std::string& nom);

    // Initialisation des parkings
    void initialiserParkings(int nombre);

    // Interface pour APP
    bool pisteLibre() const;
    bool autoriserAtterrissage(const std::string& avionId);

    // Affichage du plan de l'aéroport
    void afficherPlanAeroport() const;

    // Gestion des parkings
    std::string getParkingDisponible() const;
    void libererParking(const std::string& parkingId);
};

#endif // TWR_H