// JsonLogger.cpp
#include "../include/JSONLogger.h"
#include <iostream>
#include <ctime>

JsonLogger::JsonLogger(const std::string& nomFichier)
    : nomFichier(nomFichier), premierElement(true) {
    ouvrirFichier();
}

JsonLogger::~JsonLogger() {
    fermerFichier();
}

void JsonLogger::ouvrirFichier() {
    fichier.open(nomFichier);
    if (fichier.is_open()) {
        fichier << "{\n";
        fichier << "  \"logs\": [\n";
    }
}

void JsonLogger::fermerFichier() {
    if (fichier.is_open()) {
        fichier << "\n  ]\n";
        fichier << "}\n";
        fichier.close();
    }
}

void JsonLogger::logPosition(const Avion& avion) {
    if (!fichier.is_open()) return;

    if (!premierElement) {
        fichier << ",\n";
    }
    premierElement = false;

    Position3D pos = avion.getPosition();

    fichier << "    {\n";
    fichier << "      \"type\": \"position\",\n";
    fichier << "      \"avion\": \"" << avion.getCode() << "\",\n";
    fichier << "      \"x\": " << pos.x << ",\n";
    fichier << "      \"y\": " << pos.y << ",\n";
    fichier << "      \"altitude\": " << pos.altitude << "\n";
    fichier << "    }";
}

void JsonLogger::logMessage(const Message& msg) {
    if (!fichier.is_open()) return;

    if (!premierElement) {
        fichier << ",\n";
    }
    premierElement = false;

    fichier << "    {\n";
    fichier << "      \"type\": \"message\",\n";
    fichier << "      \"expediteur\": \"" << msg.expediteur << "\",\n";
    fichier << "      \"destinataire\": \"" << msg.destinataire << "\",\n";
    fichier << "      \"contenu\": \"" << msg.contenu << "\"\n";
    fichier << "    }";
}

void JsonLogger::logEvenement(const std::string& type, const std::string& description) {
    if (!fichier.is_open()) return;

    if (!premierElement) {
        fichier << ",\n";
    }
    premierElement = false;

    fichier << "    {\n";
    fichier << "      \"type\": \"evenement\",\n";
    fichier << "      \"categorie\": \"" << type << "\",\n";
    fichier << "      \"description\": \"" << description << "\"\n";
    fichier << "    }";
}