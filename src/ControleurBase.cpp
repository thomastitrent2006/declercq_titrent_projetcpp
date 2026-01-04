#include "../include/ControleurBase.h"
#include <chrono>
#include <iostream>

ControleurBase::ControleurBase(const std::string& _nom)
    : nom(_nom), running(false) {
    std::string logFileName = "log_" + nom + ".json";
    logFile.open(logFileName, std::ios::app);
    if (logFile.is_open()) {
        logFile << "[\n";
        logFile.flush();
    }
}

ControleurBase::~ControleurBase() {
    if (running.load()) {
        arreter();
    }

    if (logFile.is_open()) {
        logFile << "]\n";
        logFile.close();
    }
}

void ControleurBase::ajouterAvion(Avion* avion) {
    if (avion == nullptr) return;

    std::lock_guard<std::mutex> lock(mtx);
    avionsSousControle.push_back(avion);
}

void ControleurBase::retirerAvion(const std::string& avionId) {
    std::lock_guard<std::mutex> lock(mtx);
    for (size_t i = 0; i < avionsSousControle.size(); i++) {
        if (avionsSousControle[i]->getNom() == avionId) {
            avionsSousControle.erase(avionsSousControle.begin() + i);
            break;
        }
    }
}

std::vector<Avion*> ControleurBase::getAvions() const {
    std::lock_guard<std::mutex> lock(mtx);
    return avionsSousControle;
}

void ControleurBase::envoyerMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    historiqueMessages.push_back(msg);
    logMessage(msg);
}

std::vector<Message> ControleurBase::getMessagesRecus() const {
    std::lock_guard<std::mutex> lock(mtx);
    return historiqueMessages;
}

// ? SUPPRIME LE LOCK ICI - C'est la cause du deadlock !
void ControleurBase::logMessage(const Message& msg) {
    // PAS DE LOCK - appelé depuis des fonctions qui ont déjà le lock
    if (logFile.is_open()) {
        logFile << msg.toJSON() << ",\n";
        logFile.flush();
    }
}

void ControleurBase::logAction(const std::string& action, const std::string& details) {
    // PAS DE LOCK ICI NON PLUS
    Message msg;
    msg.expediteur = nom;
    msg.destinataire = "LOG";
    msg.type = action;
    msg.contenu = details;
    msg.avionId = "";
    msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    logMessage(msg);
}

void ControleurBase::demarrer() {
    bool expected = false;
    if (!running.compare_exchange_strong(expected, true)) {
        std::cout << "[" << nom << "] Déjà démarré\n";
        return;
    }

    try {
        workerThread = std::thread([this]() {
            std::cout << "[" << nom << "] Thread démarré\n";

            while (running.load()) {
                try {
                    processLogic();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                catch (const std::exception& e) {
                    std::cerr << "[" << nom << "] Erreur dans processLogic: " << e.what() << "\n";
                }
                catch (...) {
                    std::cerr << "[" << nom << "] Erreur inconnue dans processLogic\n";
                }
            }

            std::cout << "[" << nom << "] Thread arrêté\n";
            });
    }
    catch (const std::system_error& e) {
        std::cerr << "[" << nom << "] ERREUR CRÉATION THREAD: " << e.what() << "\n";
        running.store(false);
        throw;
    }
}

void ControleurBase::arreter() {
    bool expected = true;
    if (!running.compare_exchange_strong(expected, false)) {
        return;
    }

    if (workerThread.joinable()) {
        try {
            workerThread.join();
        }
        catch (const std::system_error& e) {
            std::cerr << "[" << nom << "] Erreur lors de l'arrêt du thread: " << e.what() << "\n";
        }
    }
}

std::string Message::toJSON() const {
    return "{"
        "\"expediteur\":\"" + expediteur + "\","
        "\"destinataire\":\"" + destinataire + "\","
        "\"type\":\"" + type + "\","
        "\"avionId\":\"" + avionId + "\","
        "\"contenu\":\"" + contenu + "\","
        "\"timestamp\":" + std::to_string(timestamp) +
        "}";
}