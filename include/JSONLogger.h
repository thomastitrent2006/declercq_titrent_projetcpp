// JsonLogger.h
#ifndef JSON_LOGGER_H
#define JSON_LOGGER_H

#include <string>
#include <fstream>
#include <vector>
#include "Avion.h"
#include "Message.h"

class JsonLogger {
private:
    std::string nomFichier;
    std::ofstream fichier;
    bool premierElement;

public:
    JsonLogger(const std::string& nomFichier);
    ~JsonLogger();

    void ouvrirFichier();
    void fermerFichier();

    void logPosition(const Avion& avion);
    void logMessage(const Message& msg);
    void logEvenement(const std::string& type, const std::string& description);
};

#endif