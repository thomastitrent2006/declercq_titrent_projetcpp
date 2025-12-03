// DonneesPartagees.cpp
#include "../include/DonneesPartagees.h"

void DonneesPartagees::ajouterAvion(Avion* avion) {
    std::lock_guard<std::mutex> lock(mutexAvions);
    avions.push_back(avion);
}

void DonneesPartagees::ajouterMessage(const Message& msg) {
    std::lock_guard<std::mutex> lock(mutexMessages);
    messages.push_back(msg);
}