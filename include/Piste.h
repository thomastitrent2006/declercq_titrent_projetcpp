// Piste.h
#ifndef PISTE_H
#define PISTE_H

#include <string>

enum class EtatPiste {
    LIBRE,
    OCCUPEE
};

class Piste {
private:
    std::string identifiant;
    EtatPiste etat;

public:
    Piste(const std::string& id);

    bool estLibre() const { return etat == EtatPiste::LIBRE; }
    void occuper() { etat = EtatPiste::OCCUPEE; }
    void liberer() { etat = EtatPiste::LIBRE; }
};

#endif