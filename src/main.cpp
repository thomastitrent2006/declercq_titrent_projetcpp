#include "../include/Avion.h"
#include <iostream>
#include <ctime>
#include <vector>
#include <SFML/Graphics.hpp>
#include <cmath>

using namespace sf;

const int WINDOW_SIZE_X = 1200;
const int WINDOW_SIZE_Y = 1104;

#ifdef _MSC_VER
#define _PATHIMG "C:/SFML_3.0.2/img/"
#else
#define _PATHIMG "./img/"
#endif

// Fonction pour convertir les coordonnées monde en coordonnées écran
Vector2f worldToScreen(const Position& posAvion, const Position& depart, const Position& arrivee,
    const Vector2f& screenStart, const Vector2f& screenEnd) {
    double total_dx = arrivee.x - depart.x;
    double total_dy = arrivee.y - depart.y;

    double current_dx = posAvion.x - depart.x;
    double current_dy = posAvion.y - depart.y;

    if (total_dx == 0) total_dx = 1;
    if (total_dy == 0) total_dy = 1;

    double ratio_x = current_dx / total_dx;
    double ratio_y = current_dy / total_dy;

    float screen_x = screenStart.x + ratio_x * (screenEnd.x - screenStart.x);
    float screen_y = screenStart.y + ratio_y * (screenEnd.y - screenStart.y);

    return Vector2f(screen_x, screen_y);
}

void initializeSimulation() {
    RenderWindow window(VideoMode({ WINDOW_SIZE_X, WINDOW_SIZE_Y }), "Air Traffic Control");
    window.setFramerateLimit(60);

    // Charger la carte
    Texture backgroundImage;
    if (!backgroundImage.loadFromFile(std::string(_PATHIMG) + "france.png")) {
        std::cerr << "Erreur chargement carte" << std::endl;
        return;
    }
    Sprite backgroundSprite(backgroundImage);

    std::vector<Sprite> airportSprites;

    // Charger l'aéroport
    Texture aeroportImage;
    if (!aeroportImage.loadFromFile(std::string(_PATHIMG) + "airport.png")) {
        std::cerr << "Erreur chargement airport" << std::endl;
        return;
    }
    Sprite airport1(aeroportImage);
    airport1.scale({ 0.3f, 0.3f });
    airport1.setPosition({ 600, 80 });
    airportSprites.push_back(airport1);

    Sprite airport2(aeroportImage);
    airport2.scale({ 0.3f, 0.3f });
    airport2.setPosition({ 560, 280 });
    airportSprites.push_back(airport2);

    Sprite airport3(aeroportImage);
    airport3.scale({ 0.3f, 0.3f });
    airport3.setPosition({ 470, 780 });
    airportSprites.push_back(airport3);

    Sprite airport4(aeroportImage);
    airport4.scale({ 0.3f, 0.3f });
    airport4.setPosition({ 280, 450 });
    airportSprites.push_back(airport4);

    Sprite airport5(aeroportImage);
    airport5.scale({ 0.3f, 0.3f });
    airport5.setPosition({ 750, 620 });
    airportSprites.push_back(airport5);

    Sprite airport6(aeroportImage);
    airport6.scale({ 0.2f, 0.2f });
    airport6.setPosition({ 800, 850 });
    airportSprites.push_back(airport6);

    // Charger l'avion
    Texture airplane;
    if (!airplane.loadFromFile(std::string(_PATHIMG) + "airplane.png")) {
        std::cerr << "Erreur chargement avion" << std::endl;
        return;
    }
    Sprite Spriteairplane(airplane);
    Spriteairplane.scale({ 0.15f, 0.15f });

    // Positions
    Vector2f positionDepartEcran(WINDOW_SIZE_X / 2.0f, WINDOW_SIZE_Y / 2.0f);
    Vector2f positionArriveeEcran(600, 80);

    Position depart(0, 0, 0);
    Position arrivee(100000, 50000, 0);
    Avion avion("AF123", depart, arrivee);

    Spriteairplane.setPosition(positionDepartEcran);

    Clock clock;

    // Boucle principale
    while (window.isOpen() && !avion.volTermine()) {
        while (const std::optional<Event> event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
                event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
                event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Mise à jour de l'avion avec simulation accélérée
        avion.update(deltaTime * 50);

        // Mise à jour de la position du sprite
        Position posActuelle = avion.getPosition();
        Vector2f screenPos = worldToScreen(posActuelle, depart, arrivee,
            positionDepartEcran, positionArriveeEcran);
        Spriteairplane.setPosition(screenPos);

        // Rendu
        window.clear();
        window.draw(backgroundSprite);
        // Dessiner tous les aéroports
        for (const auto& airport : airportSprites) {
            window.draw(airport);
        }
        
        window.draw(Spriteairplane);
        window.display();
    }
}

int main() {
    initializeSimulation();
    return 0;
}