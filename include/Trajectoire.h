// Trajectoire.h
#ifndef TRAJECTOIRE_H
#define TRAJECTOIRE_H

#include <SFML/Graphics.hpp>
#include "Avion.h"

// Classe pour gérer la trajectoire et l'affichage d'un avion
class Trajectoire {
private:
    Position depart;
    Position arrivee;
    sf::Vector2f screenStart;
    sf::Vector2f screenEnd;
    float angle;  // Angle de rotation de l'avion

public:
    // Constructeur
    Trajectoire(const Position& dep, const Position& arr,
        const sf::Vector2f& screenDep, const sf::Vector2f& screenArr);

    // Convertir les coordonnées monde en coordonnées écran
    sf::Vector2f worldToScreen(const Position& pos) const;

    // Mettre à jour le sprite de l'avion
    void updateSprite(sf::Sprite& sprite, Avion& avion);

    // Getters
    float getAngle() const { return angle; }
    Position getDepart() const { return depart; }
    Position getArrivee() const { return arrivee; }
};

#endif // TRAJECTOIRE_H