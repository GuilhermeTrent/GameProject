#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <SFML/Graphics.hpp>
#include "Scene.h"
#include "GameProject.h"

class SplashScreen : public Scene {
private:
    sf::Text _splashText;
    sf::Text _continueText;
    bool _readyToTransition;
    float _elapsedTime;

    
    float _textPulseTime;
    const float _pulseRate = 0.8f; // How fast the text pulses

public:
    SplashScreen(GameEngine* gameEngine);
    void sRender() override;
    void update(sf::Time dt) override;
    void sDoAction(const Command& action) override;
    void init();

    
    void onEnd() override {}  
};

#endif // SPLASHSCREEN_H