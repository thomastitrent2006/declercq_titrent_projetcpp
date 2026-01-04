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
    Position posCDG = screenToWorld(Vector2f(600.0f, 80.0f), WINDOW_SIZE_X, WINDOW_SIZE_Y);
    TWR* twrCDG = new TWR("TWR_CDG");
    APP* appCDG = new APP("APP_CDG", posCDG, 50000.0f, twrCDG, ccr);
    ccr->ajouterAeroport("CDG", posCDG, appCDG, 15);

    Position posORY = screenToWorld(Vector2f(560.0f, 280.0f), WINDOW_SIZE_X, WINDOW_SIZE_Y);
    TWR* twrORY = new TWR("TWR_ORY");
    APP* appORY = new APP("APP_ORY", posORY, 45000.0f, twrORY, ccr);
    ccr->ajouterAeroport("ORY", posORY, appORY, 12);

    Position posMRS = screenToWorld(Vector2f(470.0f, 780.0f), WINDOW_SIZE_X, WINDOW_SIZE_Y);
    TWR* twrMRS = new TWR("TWR_MRS");
    APP* appMRS = new APP("APP_MRS", posMRS, 40000.0f, twrMRS, ccr);
    ccr->ajouterAeroport("MRS", posMRS, appMRS, 10);

    Position posNTE = screenToWorld(Vector2f(280.0f, 450.0f), WINDOW_SIZE_X, WINDOW_SIZE_Y);
    TWR* twrNTE = new TWR("TWR_NTE");
    APP* appNTE = new APP("APP_NTE", posNTE, 35000.0f, twrNTE, ccr);
    ccr->ajouterAeroport("NTE", posNTE, appNTE, 8);

    Position posLYS = screenToWorld(Vector2f(750.0f, 620.0f), WINDOW_SIZE_X, WINDOW_SIZE_Y);
    TWR* twrLYS = new TWR("TWR_LYS");
    APP* appLYS = new APP("APP_LYS", posLYS, 40000.0f, twrLYS, ccr);
    ccr->ajouterAeroport("LYS", posLYS, appLYS, 10);

    Position posNCE = screenToWorld(Vector2f(800.0f, 850.0f), WINDOW_SIZE_X, WINDOW_SIZE_Y);
    TWR* twrNCE = new TWR("TWR_NCE");
    APP* appNCE = new APP("APP_NCE", posNCE, 38000.0f, twrNCE, ccr);
    ccr->ajouterAeroport("NCE", posNCE, appNCE, 9);

    // Créer quelques routes
    ccr->ajouterRoute("CDG", "MRS");
    ccr->ajouterRoute("CDG", "NCE");
    ccr->ajouterRoute("ORY", "LYS");
    ccr->ajouterRoute("NTE", "MRS");

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
        appCDG->processLogic();
        appORY->processLogic();
        appMRS->processLogic();
        appNTE->processLogic();
        appLYS->processLogic();
        appNCE->processLogic();

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
    delete ccr;
    delete appCDG; delete twrCDG;
    delete appORY; delete twrORY;
    delete appMRS; delete twrMRS;
    delete appNTE; delete twrNTE;
    delete appLYS; delete twrLYS;
    delete appNCE; delete twrNCE;
}

int main() {
    initializeSimulation();
    return 0;
}