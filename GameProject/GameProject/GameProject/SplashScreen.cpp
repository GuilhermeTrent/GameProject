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

//SplashScreen::SplashScreen(sf::RenderWindow& win) : window(win) {
//    /*if (!pugTexture.loadFromFile("pug_running.png")) {
//        std::cerr << "Error loading pug image!" << std::endl;
//    }*/
//
//
//    pugSprite.setTexture(pugTexture);
//    pugSprite.setScale(0.5f, 0.5f);
//    pugSprite.setPosition(-100, window.getSize().y / 2 - 50);
//
//    if (!font.loadFromFile("Sansation.ttf")) {
//        std::cerr << "Error loading font!" << std::endl;
//    }
//
//    titleText.setFont(Assets::getInstance().getFont("main"));
//    titleText.setString("🏁 Welcome to PUG RACE! 🏁");
//    titleText.setCharacterSize(40);
//    titleText.setFillColor(sf::Color::Black);
//    titleText.setPosition(50, 50);
//
//    infoText.setFont(font);
//    infoText.setString("Ever seen a tiny, squishy-faced speed demon zoom past?\nThat's a Pug Race! 🐶💨");
//    infoText.setCharacterSize(24);
//    infoText.setFillColor(sf::Color::Black);
//    infoText.setPosition(50, 120);
//
//    startText.setFont(font);
//    startText.setString("Press ENTER to Start");
//    startText.setCharacterSize(24);
//    startText.setFillColor(sf::Color::Red);
//    startText.setPosition(50, window.getSize().y - 50);
//}
//
//void SplashScreen::update(float deltaTime) {
//    if (pugSprite.getPosition().x < window.getSize().x) {
//        pugSprite.move(pugSpeed * deltaTime, 0);
//    }
//}
//
//void SplashScreen::draw() {
//    window.clear(sf::Color(255, 223, 186));
//    window.draw(titleText);
//    window.draw(infoText);
//    window.draw(pugSprite);
//    window.draw(startText);
//    window.display();
//}
//
//bool SplashScreen::handleInput() {
//    sf::Event event;
//    while (window.pollEvent(event)) {
//        if (event.type == sf::Event::Closed) {
//            window.close();
//        }
//        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
//            return true;
//        }
//    }
//    return false;
//}

SplashScreen::SplashScreen(GameEngine* gameEngine)
    : Scene(gameEngine), _elapsedTime(0.f)
{
    // Set up the splash text
    _splashText.setFont(Assets::getInstance().getFont("main"));
    _splashText.setString("Welcome to the Ultimate Pug Race!\n\nBark, Dash, and Race to Glory!\n\nGet Ready...");
    _splashText.setCharacterSize(30);
    _splashText.setFillColor(sf::Color::White);
    _splashText.setPosition(200.f, 300.f);
}

void SplashScreen::update(sf::Time dt)
{
    _elapsedTime += dt.asSeconds();
    if (_elapsedTime > 3.0f) {
        // Create the Menu scene as a shared pointer (if you have a Scene_Menu class)
        auto menuScene = std::make_shared<Scene_Menu>(_game);

        // Transition to the Menu scene
        _game->changeScene("Menu", menuScene, true);  // true to end the current scene
    }
}

void SplashScreen::sRender()
{
    _game->window().clear(sf::Color::Black);
    _game->window().draw(_splashText);
    _game->window().display();
}

