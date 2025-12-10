#include "../include/Avion.h"
#include "../include/Trajectoire.h"
#include <iostream>
#include <ctime>
#include <vector>
#include <SFML/Graphics.hpp>

using namespace sf;

const int WINDOW_SIZE_X = 1200;
const int WINDOW_SIZE_Y = 1104;

#ifdef _MSC_VER
#define _PATHIMG "C:/SFML_3.0.2/img/"
#else
#define _PATHIMG "./img/"
#endif

void initializeSimulation() {
    RenderWindow window(VideoMode({ WINDOW_SIZE_X, WINDOW_SIZE_Y }), "Air Traffic Control");
    window.setFramerateLimit(120);

    // Charger la carte
    Texture backgroundImage;
    if (!backgroundImage.loadFromFile(std::string(_PATHIMG) + "france.png")) {
        std::cerr << "Erreur chargement carte" << std::endl;
        return;
    }
    Sprite backgroundSprite(backgroundImage);

    // Charger les aéroports 
    Texture aeroportImage;
    if (!aeroportImage.loadFromFile(std::string(_PATHIMG) + "airport.png")) {
        std::cerr << "Erreur chargement airport" << std::endl;
        return;
    }
    Sprite airportSprite(aeroportImage);
    airportSprite.scale({ 0.3f, 0.3f });
    airportSprite.setPosition({ 600, 80 });

    // Charger l'avion
    Texture airplane;
    if (!airplane.loadFromFile(std::string(_PATHIMG) + "airplane.png")) {
        std::cerr << "Erreur chargement avion" << std::endl;
        return;
    }
    Sprite Spriteairplane(airplane);
    Spriteairplane.scale({ 0.15f, 0.15f });

    // Centrer l'origine du sprite
    FloatRect bounds = Spriteairplane.getLocalBounds();
    Spriteairplane.setOrigin(bounds.size.x / 2.0f, bounds.size.y / 2.0f);

    Spriteairplane.setPosition({ WINDOW_SIZE_X / 2.0f, WINDOW_SIZE_Y / 2.0f });

    // Créer l'avion et la trajectoire
    Position depart(0, 0, 0);
    Position arrivee(100000, 50000, 0);
    Avion avion("AF123", depart, arrivee);

    Vector2f positionDepartEcran(WINDOW_SIZE_X / 2.0f, WINDOW_SIZE_Y / 2.0f);
    Vector2f positionArriveeEcran(600, 80);

    // Créer l'objet Trajectoire
    Trajectoire trajectoire(depart, arrivee, positionDepartEcran, positionArriveeEcran);

    Clock clock;

    std::cout << "========================================" << std::endl;
    std::cout << "   SIMULATION DE VOL - " << avion.getNom() << std::endl;
    std::cout << "========================================" << std::endl;

    while (window.isOpen() && !avion.volTermine()) {
        while (const std::optional<Event> event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
                event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
                event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // Mise à jour de la simulation
        float deltaTime = clock.restart().asSeconds();
        avion.update(deltaTime * 10); // Facteur de vitesse pour accélérer

        // Mettre à jour le sprite via la trajectoire
        trajectoire.updateSprite(Spriteairplane, avion);

        // Affichage (optionnel pour debug)
        // std::cout << "Angle: " << trajectoire.getAngle() << " degrés" << std::endl;

        window.clear();
        window.draw(backgroundSprite);
        window.draw(airportSprite);
        window.draw(Spriteairplane);
        window.display();
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "   FIN DE LA SIMULATION" << std::endl;
    std::cout << "========================================" << std::endl;
}

void pause(int milliseconds) {
    clock_t start = clock();
    clock_t end = start + (milliseconds * CLOCKS_PER_SEC / 1000);
    while (clock() < end) {
        // Attente active
    }
}

int main() {
    initializeSimulation();
    return 0;
}