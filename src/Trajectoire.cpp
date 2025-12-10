// Trajectoire.cpp
#include "../include/Trajectoire.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructeur
Trajectoire::Trajectoire(const Position& dep, const Position& arr,
    const sf::Vector2f& screenDep, const sf::Vector2f& screenArr)
    : depart(dep), arrivee(arr), screenStart(screenDep), screenEnd(screenArr), angle(0.0f) {
    // Calculer l'angle initial
    double dx = arrivee.x - depart.x;
    double dy = arrivee.y - depart.y;
    angle = atan2(dy, dx) * 180.0f / M_PI;
}

// Convertir les coordonnées monde en coordonnées écran
sf::Vector2f Trajectoire::worldToScreen(const Position& pos) const {
    // Calculer le ratio de progression entre départ et arrivée
    double dx = arrivee.x - depart.x;
    double dy = arrivee.y - depart.y;

    // Éviter division par zéro
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;

    double ratio_x = (pos.x - depart.x) / dx;
    double ratio_y = (pos.y - depart.y) / dy;

    // Interpoler entre les positions écran
    float screen_x = screenStart.x + ratio_x * (screenEnd.x - screenStart.x);
    float screen_y = screenStart.y + ratio_y * (screenEnd.y - screenStart.y);

    return sf::Vector2f(screen_x, screen_y);
}

// Mettre à jour le sprite de l'avion
void Trajectoire::updateSprite(sf::Sprite& sprite, Avion& avion) {
    // Convertir la position monde en position écran
    Position currentPos = avion.getPosition();
    sf::Vector2f screenPos = worldToScreen(currentPos);
    sprite.setPosition(screenPos);

    // Calculer et mettre à jour l'angle de rotation
    double dx = arrivee.x - currentPos.x;
    double dy = arrivee.y - currentPos.y;
    angle = atan2(dy, dx) * 180.0f / M_PI;
    sprite.setRotation(angle);
}