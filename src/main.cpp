#include "../include/Avion.h"
#include "../include/CCR.h"
#include "../include/APP.h"
#include "../include/TWR.h"
#include <iostream>
#include <vector>
#include <thread>
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

    // ========== VECTEURS POUR STOCKER LES INSTANCES ==========
    std::vector<Avion*> planes;
    std::vector<APP*> airports;
    std::vector<TWR*> towers;
    std::vector<std::thread> planeThreads;

    // ========== POSITIONS DES AÉROPORTS EN COORDONNÉES ÉCRAN ==========
    Vector2f screenLille(600, 80);
    Vector2f screenNantes(280, 450);
    Vector2f screenToulouse(470, 780);

    // Convertir en coordonnées monde
    Position posLille = screenToWorld(screenLille, WINDOW_SIZE_X, WINDOW_SIZE_Y);
    Position posNantes = screenToWorld(screenNantes, WINDOW_SIZE_X, WINDOW_SIZE_Y);
    Position posToulouse = screenToWorld(screenToulouse, WINDOW_SIZE_X, WINDOW_SIZE_Y);

    // Altitude de croisière
    posLille.altitude = 10000.0;
    posNantes.altitude = 10000.0;
    posToulouse.altitude = 10000.0;

    // ========== CRÉER LE CCR ==========
    CCR* ccr = new CCR("CCR_France", 10000.0);

    // ========== CRÉER LES TOURS ET AÉROPORTS ==========

    // Lille
    TWR* twrLille = new TWR("TWR_Lille");
    twrLille->initialiserParkings(10);
    towers.push_back(twrLille);

    APP* appLille = new APP("APP_Lille", posLille, 50000.0, twrLille, ccr);
    airports.push_back(appLille);

    // Nantes
    TWR* twrNantes = new TWR("TWR_Nantes");
    twrNantes->initialiserParkings(10);
    towers.push_back(twrNantes);

    APP* appNantes = new APP("APP_Nantes", posNantes, 50000.0, twrNantes, ccr);
    airports.push_back(appNantes);

    // Toulouse
    TWR* twrToulouse = new TWR("TWR_Toulouse");
    twrToulouse->initialiserParkings(10);
    towers.push_back(twrToulouse);

    APP* appToulouse = new APP("APP_Toulouse", posToulouse, 50000.0, twrToulouse, ccr);
    airports.push_back(appToulouse);

    // ========== AJOUTER LES AÉROPORTS AU CCR ==========
    ccr->ajouterAeroport("Lille", posLille, appLille, 5);
    ccr->ajouterAeroport("Nantes", posNantes, appNantes, 5);
    ccr->ajouterAeroport("Toulouse", posToulouse, appToulouse, 5);

    // ========== CRÉER LES ROUTES ==========
    ccr->ajouterRoute("Lille", "Nantes");
    ccr->ajouterRoute("Nantes", "Toulouse");
    ccr->ajouterRoute("Toulouse", "Lille");

    // ========== CRÉER LES AVIONS ==========

    // Avion 1: Lille -> Nantes
    Avion* p1 = new Avion("AF123", posLille, posNantes);
    planes.push_back(p1);
    ccr->ajouterAvion(p1);

     // Avion 2: Toulouse -> Lille (optionnel)
     Avion* p2 = new Avion("LH456", posToulouse, posLille);
     planes.push_back(p2);
     ccr->ajouterAvion(p2);

    // ========== DÉMARRER LES AVIONS DANS DES THREADS ==========
    for (auto* plane : planes) {
        planeThreads.emplace_back([plane]() {
            plane->demarrer();
            });
    }
    
    std::cout << "\n=== DEMARRAGE DE LA SIMULATION ===\n";

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
            std::cout << " Avion " << plane->getNom() << " demarre\n";
        }
    }

    // ========== ÉTAPE 1 : TESTER LE CCR UNIQUEMENT ==========
    std::cout << "\n--- Test CCR ---\n";
    try {
        ccr->demarrer();
        std::cout << " CCR demarre avec succes\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Laisser le temps de démarrer
    }
    catch (const std::exception& e) {
        std::cerr << " ERREUR CCR: " << e.what() << "\n";
        std::cerr << "   → Arret de la simulation\n";
        return;  // Arrêter si le CCR ne démarre pas
    }
   
    std::cout << "\n--- Test APP ---\n";
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
    std::cout << "\n--- Test TWR ---\n";
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
    
    std::cout << "=== SIMULATION DEMARREE ===\n";
    std::cout << "Avions crees: " << planes.size() << "\n";
    std::cout << "Aeroports: " << airports.size() << "\n";
    std::cout << "Tours: " << towers.size() << "\n\n";

    // ========== CHARGER LES TEXTURES ==========

    // Carte
    Texture backgroundImage;
    if (!backgroundImage.loadFromFile(std::string(_PATHIMG) + "france.png")) {
        std::cerr << "Erreur chargement carte" << std::endl;
        return;
    }
    Sprite backgroundSprite(backgroundImage);

    // Aéroports
    Texture aeroportImage;
    if (!aeroportImage.loadFromFile(std::string(_PATHIMG) + "airport.png")) {
        std::cerr << "Erreur chargement airport" << std::endl;
        return;
    }

    std::vector<Sprite> airportSprites;

    Sprite airport1(aeroportImage); /*Lille*/
    airport1.scale({ 0.2f, 0.2f });
    airport1.setPosition(screenLille);
    airportSprites.push_back(airport1);

    Sprite airport2(aeroportImage); /*Nantes*/
    airport2.scale({ 0.2f, 0.2f });
    airport2.setPosition(screenNantes);
    airportSprites.push_back(airport2);

    Sprite airport3(aeroportImage); /*Toulouse*/
    airport3.scale({ 0.2f, 0.2f });
    airport3.setPosition(screenToulouse);
    airportSprites.push_back(airport3);

    // Avion
    Texture airplane;
    if (!airplane.loadFromFile(std::string(_PATHIMG) + "airplane.png")) {
        std::cerr << "Erreur chargement avion" << std::endl;
        return;
    }

    // Créer un sprite pour chaque avion
    std::vector<Sprite> planeSprites;
    for (size_t i = 0; i < planes.size(); i++) {
        Sprite planeSprite(airplane);
        planeSprite.scale({ 0.15f, 0.15f });
        planeSprites.push_back(planeSprite);
    }

    Clock clock;

    // ========== BOUCLE PRINCIPALE SFML ==========
    while (window.isOpen()) {
        // ========== GESTION DES ÉVÉNEMENTS ==========
        while (const std::optional<Event> event = window.pollEvent()) {
            if ((event->is<sf::Event::KeyPressed>() &&
                event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) ||
                event->is<sf::Event::Closed>()) {

                // Arrêter tous les avions avant de fermer
                for (auto* plane : planes) {
                    plane->arreter();
                }
                for (auto& thread : planeThreads) {
                    if (thread.joinable()) {
                        thread.join();
                    }
                }

                // Arrêter tous les contrôleurs
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

        // ========== LA LOGIQUE TOURNE DÉJÀ DANS LES THREADS ==========
        // Les contrôleurs (CCR, APP, TWR) tournent automatiquement via demarrer()
        // Pas besoin d'appeler processLogic() ici !

        // ========== METTRE À JOUR LES POSITIONS DES SPRITES D'AVIONS ==========
        // Boucler sur tous les avions
        for (size_t i = 0; i < planes.size(); i++) {
            Position posAvion = planes[i]->getPosition();

            // Pour l'avion Lille->Nantes (AF123)
            if (i == 0) {
                Vector2f screenPos = worldToScreen(posAvion, posLille, posNantes,
                    screenLille, screenNantes);
                planeSprites[i].setPosition(screenPos);
            }
            // Pour un éventuel deuxième avion Toulouse->Lille
            else if (i == 1) {
                Vector2f screenPos = worldToScreen(posAvion, posToulouse, posLille,
                    screenToulouse, screenLille);
                planeSprites[i].setPosition(screenPos);
            }
            // Ajouter d'autres conditions pour plus d'avions...
        }

        // ========== RENDU ==========
        window.clear(Color::Black);

        // 1. Dessiner la carte de fond
        window.draw(backgroundSprite);

        // 2. Boucler et dessiner tous les aéroports
        for (const auto& airportSprite : airportSprites) {
            window.draw(airportSprite);
        }

        // 3. Boucler et dessiner tous les avions
        for (const auto& planeSprite : planeSprites) {
            window.draw(planeSprite);
        }

        // Afficher tout à l'écran
        window.display();
    }

    // ========== NETTOYAGE ==========

    // Arrêter les avions
    for (auto* plane : planes) {
        plane->arreter();
    }

    // Attendre les threads
    for (auto& thread : planeThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Libérer la mémoire
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