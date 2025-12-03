// Message.h
#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

enum class TypeMessage {
    ENTREE_ZONE,
    DEMANDE_TRAJECTOIRE,
    AUTORISATION_ATTERRISSAGE,
    AUTORISATION_DECOLLAGE,
    DEMANDE_PISTE,
    PISTE_LIBRE,
    URGENCE
};

struct Message {
    TypeMessage type;
    std::string expediteur;
    std::string destinataire;
    std::string contenu;

    Message(TypeMessage t, const std::string& exp, const std::string& dest, const std::string& cont = "")
        : type(t), expediteur(exp), destinataire(dest), contenu(cont) {
    }
};

#endif