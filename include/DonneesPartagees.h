// DonneesPartagees.h
#ifndef DONNEES_PARTAGEES_H
#define DONNEES_PARTAGEES_H

#include <vector>
#include <mutex>
#include "Avion.h"
#include "Message.h"

struct DonneesPartagees {
    std::vector<Avion*> avions;
    std::vector<Message> messages;
    std::mutex mutexAvions;
    std::mutex mutexMessages;

    void ajouterAvion(Avion* avion);
    void ajouterMessage(const Message& msg);
};

#endif