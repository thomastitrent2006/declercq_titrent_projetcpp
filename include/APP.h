#pragma once
#include "ControleurBase.h"
#include "../include/Position.h"
#include "Avion.h"
#include <vector>
#include <queue>
#include <string>
#include <mutex>

// Déclaration forward pour éviter les dépendances circulaires
class TWR;
class CCR;

class APP : public ControleurBase {
private:
    Position centreAeroport;
    float rayonControle;

    std::vector<Avion*> avionsEnApproche;
    std::queue<std::string> fileAttenteAtterrissage;

    TWR* towerReference;
    CCR* ccrReference;

    void gererDeparts();  

public:
    // Constructeur
    APP(const std::string& nom, const Position& centre, float rayon,
        TWR* twr = nullptr, CCR* ccr = nullptr);

    // Méthode principale héritée de ControleurBase
    void processLogic() override;

    // Getters
    float getRayon() const { return rayonControle; }
    Position getPosition() const { return centreAeroport; }
    TWR* getTWR() const { return towerReference; }
    CCR* getCCR() const { return ccrReference; }
    std::vector<Avion*>& getAvionsEnApproche() { return avionsEnApproche; }

    // Setters
    void setTWR(TWR* twr) { towerReference = twr; }
    void setCCR(CCR* ccr) { ccrReference = ccr; }

    // Vérification de présence dans la zone
    bool estDansZone(const Avion& avion) const;
    bool estDansZone(const Position& pos) const;

    // Gestion des avions
    void ajouterAvionEnApproche(Avion* avion);
    void retirerAvionEnApproche(Avion* avion);
    void transfererAvionVersCCR(Avion* avion);

    // Logique de contrôle
    void gererNouvellesArrivees();
   /* void gererUrgences();*/
    void gererTrajectoires();
    void assignerTrajectoireCirculaire(Avion* avion, int niveau);
    bool demanderAutorisationAtterrissage(const std::string& avionId);

    // Affichage
    void afficherConsole() const;

    // Demander un nouveau contrôleur APP si saturé
    APP* demanderNouvelAPP();
};