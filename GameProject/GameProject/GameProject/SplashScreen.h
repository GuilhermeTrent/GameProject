// SplashScreen.h
#pragma once

#include "Scene.h"
#include <SFML/Graphics.hpp>

class SplashScreen : public Scene
{
public:
    SplashScreen(GameEngine* gameEngine);
    void init();
    void update(sf::Time dt) override;
    void sRender() override;
    void sDoAction(const Command& action) override;

private:
    // Timing properties
    float _elapsedTime;
    float _textPulseTime;
    float _pulseRate;
    bool _readyToTransition;

    // Graphics resources
    sf::Text _mainFont;

    // UI elements
    sf::RectangleShape _background;
    sf::RectangleShape _controlsPanel;
    sf::Text _titleText;
    sf::Text _taglineText;
    sf::Text _controlsTitle;
    sf::Text _controlsText;
    sf::Text _storyText;
    sf::Text _continueText;

    void onEnd() override {};
};