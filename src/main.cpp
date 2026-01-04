#include "../include/Avion.h"
#include "../include/CCR.h"
#include "../include/APP.h"
#include "../include/TWR.h"
#include <iostream>
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

// Fonction pour convertir les coordonnées écran en coordonnées monde
Position screenToWorld(const sf::Vector2f& screenPos, int windowWidth, int windowHeight) {
    const double CARTE_LARGEUR_KM = 1000.0;
    const double CARTE_HAUTEUR_KM = 950.0;

    double normX = screenPos.x / static_cast<double>(windowWidth);
    double normY = screenPos.y / static_cast<double>(windowHeight);

    double worldX = (normX - 0.5) * CARTE_LARGEUR_KM * 1000.0;
    double worldY = (normY - 0.5) * CARTE_HAUTEUR_KM * 1000.0;

    return Position(worldX, worldY, 0.0);
}

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

    // Créer le CCR
    CCR* ccr = new CCR("CCR_France", 10000.0);

    // Créer les contrôleurs d'approche et tours pour chaque aéroport
   
    
    

    // Créer quelques routes
    
    // Créer un vol de test
    ccr->creerVol("AF123", "CDG", "MRS");

    // Charger la carte
    Texture backgroundImage;
    if (!backgroundImage.loadFromFile(std::string(_PATHIMG) + "france.png")) {
        std::cerr << "Erreur chargement carte" << std::endl;
        return;
    }
    Sprite backgroundSprite(backgroundImage);

    // Charger les sprites d'aéroports
    Texture aeroportImage;
    if (!aeroportImage.loadFromFile(std::string(_PATHIMG) + "airport.png")) {
        std::cerr << "Erreur chargement airport" << std::endl;
        return;
    }

    std::vector<Sprite> airportSprites;

    Sprite airport1(aeroportImage); /*Lille*/
    airport1.scale({ 0.2f,0.2f});
    airport1.setPosition({ 600, 80 });
    airportSprites.push_back(airport1);


	Sprite airport3(aeroportImage); /*Toulouse*/
    airport3.scale({ 0.2f, 0.2f });
    airport3.setPosition({ 470, 780 });
    airportSprites.push_back(airport3);

    Sprite airport4(aeroportImage);  /*Nantes*/
    airport4.scale({ 0.2f, 0.2f });
    airport4.setPosition({ 280, 450 });
    airportSprites.push_back(airport4);
    

   

    // Charger l'avion
    Texture airplane;
    if (!airplane.loadFromFile(std::string(_PATHIMG) + "airplane.png")) {
        std::cerr << "Erreur chargement avion" << std::endl;
        return;
    }
    Sprite Spriteairplane(airplane);
    Spriteairplane.scale({ 0.15f, 0.15f });

    Clock clock;

    // Boucle principale
    while (window.isOpen()) {
        while (const std::optional<Event> event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
                event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
                event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Mettre à jour la logique du CCR
       /* ccr->processLogic();*/

        // Mettre à jour tous les APP
       
        

        // Mettre à jour toutes les tours
       /* twrCDG->processLogic();
        twrORY->processLogic();
        twrMRS->processLogic();
        twrNTE->processLogic();
        twrLYS->processLogic();
        twrNCE->processLogic();*/

        // Rendu
        window.clear();
        window.draw(backgroundSprite);

        for (const auto& airport : airportSprites) {
            window.draw(airport);
        }

        window.draw(Spriteairplane);
        window.display();
    }

    // Nettoyage
   
}

int main() {
    initializeSimulation();
    return 0;
}