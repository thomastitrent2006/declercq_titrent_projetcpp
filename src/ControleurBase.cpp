#include "../include/ControleurBase.h"
#include <chrono>
#include <sstream>
#include <algorithm>
#include <iomanip>

std::string Message::toJSON() const {
    std::ostringstream oss;
    oss << "{"
        << "\"expediteur\":\"" << expediteur << "\","
        << "\"destinataire\":\"" << destinataire << "\","
        << "\"type\":\"" << type << "\","
        << "\"avionId\":\"" << avionId << "\","
        << "\"contenu\":\"" << contenu << "\","
        << "\"timestamp\":" << timestamp
        << "}";
    return oss.str();
}

ControleurBase::ControleurBase(const std::string& _nom)
    : nom(_nom), running(false) {
    // Ouvrir le fichier de log
    std::string filename = "log_" + nom + ".json";
    logFile.open(filename, std::ios::app);
    if (logFile.is_open()) {
        logFile << "{\"controleur\":\"" << nom << "\",\"logs\":[\n";
    }
}

ControleurBase::~ControleurBase() {
    arreter();
    if (logFile.is_open()) {
        logFile << "\n]}\n";
        logFile.close();
    }
}

void ControleurBase::ajouterAvion(Avion* avion) {
    std::lock_guard<std::mutex> lock(mtx);
    avionsSousControle.push_back(avion);

    std::ostringstream oss;
    oss << "Avion " << avion->getNom() << " ajouté au contrôle";
    logAction("AJOUT_AVION", oss.str());
}

void ControleurBase::retirerAvion(const std::string& avionId) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = std::remove_if(avionsSousControle.begin(), avionsSousControle.end(),
        [&avionId](Avion* a) { return a->getNom() == avionId; });

    if (it != avionsSousControle.end()) {
        avionsSousControle.erase(it, avionsSousControle.end());
        logAction("RETRAIT_AVION", "Avion " + avionId + " retiré du contrôle");
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

void ControleurBase::logMessage(const Message& msg) {
    if (logFile.is_open()) {
        logFile << msg.toJSON() << ",\n";
        logFile.flush();
    }
}

void ControleurBase::logAction(const std::string& action, const std::string& details) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    if (logFile.is_open()) {
        logFile << "{\"action\":\"" << action << "\","
            << "\"details\":\"" << details << "\","
            << "\"timestamp\":" << timestamp << "},\n";
        logFile.flush();
    }
}

void ControleurBase::demarrer() {
    running = true;
    workerThread = std::thread([this]() {
        while (running) {
            processLogic();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        });
}

void ControleurBase::arreter() {
    running = false;
    if (workerThread.joinable()) {
        workerThread.join();
    }
}