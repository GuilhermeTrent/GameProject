#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <SFML/Graphics.hpp>
#include "Scene.h"
#include "GameProject.h"

class SplashScreen : public Scene{
private:
    sf::Text _titleText;
    sf::Text _descriptionText;
    sf::Clock _timer;
    float _displayDuration = 3.0f; // Time before switching to menu

    float _elapsedTime;
    sf::Text _splashText;


public:
    SplashScreen(GameEngine* gameEngine);
    void sRender() override;
    void update(sf::Time dt) override;
};

#endif // SPLASHSCREEN_H
