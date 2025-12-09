#include "../include/Avion.h"
#include <iostream>
#include <ctime>
#include <vector>
#include <SFML/Graphics.hpp>
using namespace sf;

const int WINDOW_SIZE_X = 1200;
const int WINDOW_SIZE_Y = 1104;

#ifdef _MSC_VER
// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
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
    airportSprite.scale({ 0.3f,0.3f });
    airportSprite.setPosition({600,80});



    while (window.isOpen()) {
        while (const std::optional<Event> event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
                event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
                event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();
        window.draw(backgroundSprite);
        window.draw(airportSprite);
        window.display();
    }
  
}


// Fonction de pause simple (sans threads)
void pause(int milliseconds) {
    clock_t start = clock();
    clock_t end = start + (milliseconds * CLOCKS_PER_SEC / 1000);
    while (clock() < end) {
        // Attente active
    }
}

int main() {
    initializeSimulation(); 

    // Création de l'avion
    Position depart(0, 0, 0);
    Position arrivee(100000, 50000, 0);

    Avion avion("AF123", depart, arrivee);

    std::cout << "========================================" << std::endl;
    std::cout << "   SIMULATION DE VOL - " << avion.getNom() << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    double dt = 1.0;
    double temps_total = 0;
    int compteur_affichage = 0;

    while (!avion.volTermine() && temps_total < 3600) {
        avion.update(dt);

        if (compteur_affichage % 10 == 0) {
            std::cout << "Temps: " << temps_total << "s | ";
            avion.afficherEtat();
            std::cout << std::endl;

            // Pause d'1 seconde
            pause(1000);
        }

        temps_total += dt;
        compteur_affichage++;
    }

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "   FIN DE LA SIMULATION" << std::endl;
    std::cout << "   Temps total: " << temps_total << " secondes" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}