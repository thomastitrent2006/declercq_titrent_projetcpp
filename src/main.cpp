#include "../include/Avion.h"
#include "../include/CCR.h"
#include "../include/APP.h"
#include "../include/TWR.h"
#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <SFML/Graphics.hpp>

using namespace sf;

const int WINDOW_SIZE_X = 1200;
const int WINDOW_SIZE_Y = 1104;

#ifdef _MSC_VER
#define _PATHIMG "C:/SFML_3.0.2/img/"
#else
#define _PATHIMG "./img/"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float calculerAngleRotation(const Position& posAvion, const Position& destination) {
    double dx = destination.x - posAvion.x;
    double dy = destination.y - posAvion.y;
    double angleRadians = std::atan2(dy, dx);
    double angleDegres = angleRadians * 180.0 / M_PI;
    return static_cast<float>(angleDegres);
}

Position screenToWorld(const sf::Vector2f& screenPos, int windowWidth, int windowHeight) {
    const double CARTE_LARGEUR_KM = 1000.0;
    const double CARTE_HAUTEUR_KM = 950.0;

    double normX = screenPos.x / static_cast<double>(windowWidth);
    double normY = screenPos.y / static_cast<double>(windowHeight);

    double worldX = (normX - 0.5) * CARTE_LARGEUR_KM * 1000.0;
    double worldY = (normY - 0.5) * CARTE_HAUTEUR_KM * 1000.0;

    return Position(worldX, worldY, 0.0);
}

Vector2f worldToScreenDynamic(const Position& posAvion,
    const std::vector<Vector2f>& screenAirports,
    const std::vector<Position>& worldAirports) {

    const double MONDE_MIN_X = -500000.0;
    const double MONDE_MAX_X = 500000.0;
    const double MONDE_MIN_Y = -475000.0;
    const double MONDE_MAX_Y = 475000.0;

    double normX = (posAvion.x - MONDE_MIN_X) / (MONDE_MAX_X - MONDE_MIN_X);
    double normY = (posAvion.y - MONDE_MIN_Y) / (MONDE_MAX_Y - MONDE_MIN_Y);

    float screen_x = normX * WINDOW_SIZE_X;
    float screen_y = normY * WINDOW_SIZE_Y;

    return Vector2f(screen_x, screen_y);
}

void initializeSimulation() {
    RenderWindow window(VideoMode({ WINDOW_SIZE_X, WINDOW_SIZE_Y }), "Air Traffic Control - Rotation d'attente");
    window.setFramerateLimit(60);

    std::vector<Avion*> planes;
    std::vector<APP*> airports;
    std::vector<TWR*> towers;
    std::vector<std::thread> planeThreads;

    Vector2f screenLille(600, 80);
    Vector2f screenNantes(280, 450);
    Vector2f screenToulouse(470, 780);

    Position posLille = screenToWorld(screenLille, WINDOW_SIZE_X, WINDOW_SIZE_Y);
    Position posNantes = screenToWorld(screenNantes, WINDOW_SIZE_X, WINDOW_SIZE_Y);
    Position posToulouse = screenToWorld(screenToulouse, WINDOW_SIZE_X, WINDOW_SIZE_Y);

    posLille.altitude = 10000.0;
    posNantes.altitude = 10000.0;
    posToulouse.altitude = 10000.0;

    CCR* ccr = new CCR("CCR_France", 10000.0);

   

    // Lille 
    TWR* twrLille = new TWR("TWR_Lille");
    twrLille->initialiserParkings(1);  
    towers.push_back(twrLille);

    APP* appLille = new APP("APP_Lille", posLille, 50000.0, twrLille, ccr);
    airports.push_back(appLille);

    // Nantes 
    TWR* twrNantes = new TWR("TWR_Nantes");
    twrNantes->initialiserParkings(1);  
    towers.push_back(twrNantes);

    APP* appNantes = new APP("APP_Nantes", posNantes, 50000.0, twrNantes, ccr);
    airports.push_back(appNantes);

    // Toulouse
    TWR* twrToulouse = new TWR("TWR_Toulouse");
    twrToulouse->initialiserParkings(1);  
    towers.push_back(twrToulouse);

    APP* appToulouse = new APP("APP_Toulouse", posToulouse, 50000.0, twrToulouse, ccr);
    airports.push_back(appToulouse);

    ccr->ajouterAeroport("Lille", posLille, appLille, 1);
    ccr->ajouterAeroport("Nantes", posNantes, appNantes, 1);
    ccr->ajouterAeroport("Toulouse", posToulouse, appToulouse, 1);

    ccr->ajouterRoute("Lille", "Nantes");
    ccr->ajouterRoute("Nantes", "Toulouse");
    ccr->ajouterRoute("Toulouse", "Lille");

    std::vector<Position> toutesDestinations = { posLille, posNantes, posToulouse };

    // CRÉER PLUSIEURS AVIONS 
    Avion* p1 = new Avion("AF123", posLille, toutesDestinations);
    planes.push_back(p1);
    ccr->ajouterAvion(p1);

    Avion* p2 = new Avion("LH456", posToulouse, toutesDestinations);
    planes.push_back(p2);
    ccr->ajouterAvion(p2);

    Avion* p3 = new Avion("BA789", posNantes, toutesDestinations);
    planes.push_back(p3);
    ccr->ajouterAvion(p3);


    Avion* p4 = new Avion("EZ321", posLille, toutesDestinations);
    planes.push_back(p4);
    ccr->ajouterAvion(p4);

    Avion* p5 = new Avion("RY654", posNantes, toutesDestinations);
    planes.push_back(p5);
    ccr->ajouterAvion(p5);

    Avion::demarrerSimulation();

    for (auto* plane : planes) {
        if (plane != nullptr) {
            planeThreads.emplace_back([plane]() {
                try {
                    plane->demarrer();
                }
                catch (const std::exception& e) {
                    std::cerr << "Erreur avion " << plane->getNom() << ": " << e.what() << "\n";
                }
                });
            std::cout << "  Avion " << plane->getNom() << " demarre\n";
        }
    }

    try {
        ccr->demarrer();
        std::cout << " CCR demarre\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    catch (const std::exception& e) {
        std::cerr << " ERREUR CCR: " << e.what() << "\n";
        return;
    }

    for (size_t i = 0; i < airports.size(); i++) {
        try {
            airports[i]->demarrer();
            std::cout << " APP " << (i + 1) << " demarre\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        catch (const std::exception& e) {
            std::cerr << " ERREUR APP " << (i + 1) << ": " << e.what() << "\n";
        }
    }

    for (size_t i = 0; i < towers.size(); i++) {
        try {
            towers[i]->demarrer();
            std::cout << " TWR " << (i + 1) << " demarre\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        catch (const std::exception& e) {
            std::cerr << " ERREUR TWR " << (i + 1) << ": " << e.what() << "\n";
        }
    }

    std::cout << "\n=== SIMULATION DEMARREE ===\n";
    std::cout << "Avions: " << planes.size() << " | Aéroports: " << airports.size() << "\n\n";

    // Chargement des textures
    Texture backgroundImage;
    if (!backgroundImage.loadFromFile(std::string(_PATHIMG) + "france.png")) {
        std::cerr << "Erreur chargement carte" << std::endl;
        return;
    }
    Sprite backgroundSprite(backgroundImage);

    Texture aeroportImage;
    if (!aeroportImage.loadFromFile(std::string(_PATHIMG) + "airport.png")) {
        std::cerr << "Erreur chargement airport" << std::endl;
        return;
    }

    std::vector<Sprite> airportSprites;

    Sprite airport1(aeroportImage);
    airport1.scale({ 0.2f, 0.2f });
    airport1.setPosition(screenLille);
    airportSprites.push_back(airport1);

    Sprite airport2(aeroportImage);
    airport2.scale({ 0.2f, 0.2f });
    airport2.setPosition(screenNantes);
    airportSprites.push_back(airport2);

    Sprite airport3(aeroportImage);
    airport3.scale({ 0.2f, 0.2f });
    airport3.setPosition(screenToulouse);
    airportSprites.push_back(airport3);

    Texture airplane;
    if (!airplane.loadFromFile(std::string(_PATHIMG) + "airplane.png")) {
        std::cerr << "Erreur chargement avion" << std::endl;
        return;
    }

    std::vector<Sprite> planeSprites;
    for (size_t i = 0; i < planes.size(); i++) {
        Sprite planeSprite(airplane);
        planeSprite.scale({ 0.15f, 0.15f });

        FloatRect bounds = planeSprite.getLocalBounds();
        planeSprite.setOrigin(bounds.size / 2.0f);

        planeSprites.push_back(planeSprite);
    }

    Clock clock;

    std::vector<Vector2f> screenAirports = { screenLille, screenNantes, screenToulouse };
    std::vector<Position> worldAirports = { posLille, posNantes, posToulouse };

    while (window.isOpen()) {
        while (const std::optional<Event> event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
                event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
                event->is<sf::Event::Closed>()) {

                for (auto* plane : planes) {
                    plane->arreter();
                }
                for (auto& thread : planeThreads) {
                    if (thread.joinable()) {
                        thread.join();
                    }
                }

                ccr->arreter();
                for (auto* airport : airports) {
                    airport->arreter();
                }
                for (auto* tower : towers) {
                    tower->arreter();
                }

                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        for (size_t i = 0; i < planes.size() && i < planeSprites.size(); i++) {
            if (planes[i] != nullptr) {
                try {
                    Position posAvion = planes[i]->getPosition();
                    Position destination = planes[i]->getDestination();

                    Vector2f screenPos = worldToScreenDynamic(posAvion, screenAirports, worldAirports);
                    planeSprites[i].setPosition(screenPos);

                    float angleDegres = calculerAngleRotation(posAvion, destination);
                    planeSprites[i].setRotation(sf::degrees(angleDegres));

                    EtatAvion etat = planes[i]->getEtat();
                    if (etat == EtatAvion::PARKING) {
                        planeSprites[i].setColor(Color(150, 150, 150));
                    }
                    else if (etat == EtatAvion::CROISIERE) {
                        planeSprites[i].setColor(Color::White);
                    }
                    else if (etat == EtatAvion::DESCENTE || etat == EtatAvion::APPROCHE) {
                        planeSprites[i].setColor(Color::Yellow);
                    }
                    else if (etat == EtatAvion::MONTEE || etat == EtatAvion::DECOLLAGE) {
                        planeSprites[i].setColor(Color::Green);
                    }
                    else {
                        planeSprites[i].setColor(Color::Cyan);
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Erreur sprite avion " << i << ": " << e.what() << "\n";
                }
            }
        }

        window.clear(Color::Black);
        window.draw(backgroundSprite);

        for (const auto& airportSprite : airportSprites) {
            window.draw(airportSprite);
        }

        for (const auto& planeSprite : planeSprites) {
            window.draw(planeSprite);
        }

        window.display();
    }

    for (auto* plane : planes) {
        plane->arreter();
    }

    for (auto& thread : planeThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    for (auto* plane : planes) {
        delete plane;
    }
    for (auto* airport : airports) {
        delete airport;
    }
    for (auto* tower : towers) {
        delete tower;
    }
    delete ccr;

    std::cout << "\n=== SIMULATION TERMINÉE ===\n";
}

int main() {
    initializeSimulation();
    return 0;
}