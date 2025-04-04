

// SplashScreen.cpp
#include "SplashScreen.h"
#include "GameProject.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include "GameEngine.h"
#include "Scene_Menu.h"
#include <iostream>

SplashScreen::SplashScreen(GameEngine* gameEngine)
    : Scene(gameEngine), _elapsedTime(0.f), _readyToTransition(false), _textPulseTime(0.f)
{
    init();
}

void SplashScreen::init()
{
    // Register any key press action
    registerAction(sf::Keyboard::Space, "CONTINUE");
    registerAction(sf::Keyboard::Return, "CONTINUE");
    registerAction(sf::Keyboard::Escape, "CONTINUE");
    // You can add more keys as needed

    // Set up the splash text
    _splashText.setFont(Assets::getInstance().getFont("main"));
    _splashText.setString("Welcome to the Ultimate Pug Race!\n\nBark, Dash, and Race to Glory!");
    _splashText.setCharacterSize(30);
    _splashText.setFillColor(sf::Color::White);

    // Center the splash text
    sf::FloatRect splashBounds = _splashText.getLocalBounds();
    _splashText.setOrigin(splashBounds.width / 2.f, splashBounds.height / 2.f);
    _splashText.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f - 50.f);

    // Set up the continue text
    _continueText.setFont(Assets::getInstance().getFont("main"));
    _continueText.setString("Press any key to continue...");
    _continueText.setCharacterSize(22);
    _continueText.setFillColor(sf::Color(200, 200, 200)); // Slightly dimmer than the splash text

    // Center the continue text
    sf::FloatRect continueBounds = _continueText.getLocalBounds();
    _continueText.setOrigin(continueBounds.width / 2.f, continueBounds.height / 2.f);
    _continueText.setPosition(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f + 150.f);
}

void SplashScreen::update(sf::Time dt)
{
    _elapsedTime += dt.asSeconds();
    _textPulseTime += dt.asSeconds();

    // Create a pulsing effect for the continue text
    float alpha = 155 + 100 * std::sin(_textPulseTime * _pulseRate * 3.14159f);
    _continueText.setFillColor(sf::Color(200, 200, 200, static_cast<sf::Uint8>(alpha)));

    // Only show the continue text after a short delay
    if (_elapsedTime < 1.0f) {
        _continueText.setFillColor(sf::Color::Transparent);
    }

    // Transition to menu if ready
    if (_readyToTransition) {
        // Create the Menu scene as a shared pointer
        auto menuScene = std::make_shared<Scene_Menu>(_game);

        // Transition to the Menu scene
        _game->changeScene("Menu", menuScene, true);  // true to end the current scene
    }
}

void SplashScreen::sRender()
{
    _game->window().clear(sf::Color(25, 25, 50)); // Dark blue background

    // Draw a simple background effect
    sf::RectangleShape backgroundRect;
    backgroundRect.setSize(sf::Vector2f(_game->window().getSize().x, _game->window().getSize().y));
    backgroundRect.setFillColor(sf::Color(25, 25, 50));
    _game->window().draw(backgroundRect);

    // Draw the splash text and continue text
    _game->window().draw(_splashText);
    _game->window().draw(_continueText);
}

void SplashScreen::sDoAction(const Command& action)
{
    // Only transition if we've waited at least a second to avoid accidental skipping
    if (_elapsedTime >= 1.0f && action.type() == "START" && action.name() == "CONTINUE") {
        _readyToTransition = true;
    }
}