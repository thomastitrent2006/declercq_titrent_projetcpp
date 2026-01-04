#ifndef CONTROLEUR_BASE_H
#define CONTROLEUR_BASE_H

#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <thread>
#include <atomic>
#include "Avion.h"

// Structure pour les messages entre contrôleurs
struct Message {
    std::string expediteur;
    std::string destinataire;
    std::string type;      // "DEMANDE_ATTERRISSAGE", "AUTORISATION", etc.
    std::string avionId;
    std::string contenu;
    long timestamp = 0;

    std::string toJSON() const;
};

class ControleurBase {
protected:
    std::string nom;
    std::vector<Avion*> avionsSousControle;
    std::vector<Message> historiqueMessages;
    mutable std::mutex mtx;
    std::ofstream logFile;
    std::thread workerThread;
    std::atomic<bool> running;

    // Méthode virtuelle pure pour le traitement principal
    virtual void processLogic() = 0;

    // Enregistre un message dans le log JSON
    void logMessage(const Message& msg);
    void logAction(const std::string& action, const std::string& details);

public:
    ControleurBase(const std::string& _nom);
    virtual ~ControleurBase();

    // Gestion des avions
    void ajouterAvion(Avion* avion);
    void retirerAvion(const std::string& avionId);
    std::vector<Avion*> getAvions() const;

    // Gestion des messages
    void envoyerMessage(const Message& msg);
    std::vector<Message> getMessagesRecus() const;

    // Démarrage et arrêt du thread
    void demarrer();
    void arreter();
    
    std::string getNom() const { return nom; }
};

#endif 