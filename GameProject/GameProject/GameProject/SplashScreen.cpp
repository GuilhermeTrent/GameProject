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
#include <cmath>

SplashScreen::SplashScreen(GameEngine* gameEngine)
    : Scene(gameEngine), _elapsedTime(0.f), _readyToTransition(false), _textPulseTime(0.f), _pulseRate(1.0f)
{
    init();
}

void SplashScreen::init()
{

    MusicPlayer::getInstance().play("menuTheme");
    MusicPlayer::getInstance().setVolume(25);
    // Register any key press action
    registerAction(sf::Keyboard::Space, "CONTINUE");
    registerAction(sf::Keyboard::Return, "CONTINUE");
    registerAction(sf::Keyboard::Escape, "CONTINUE");
    // You can add more keys as needed

    // Load font
  /*  if (!_mainFont.loadFromFile("assets/fonts/main.ttf")) {
        std::cout << "Warning: Font not loaded!" << std::endl;
    }*/
    //m_menuText.setFont(Assets::getInstance().getFont("Arcade"));

    _mainFont.setFont(Assets::getInstance().getFont("Arcade"));

    // Background setup
    _background.setSize(sf::Vector2f(_game->window().getSize().x, _game->window().getSize().y));
    _background.setFillColor(sf::Color(25, 25, 50)); // Dark blue background

    // Set up the title text
    _titleText.setFont(Assets::getInstance().getFont("Arcade"));
    _titleText.setString("PUG GRAND PRIX");
    _titleText.setCharacterSize(60);
    _titleText.setFillColor(sf::Color::White);
    _titleText.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = _titleText.getLocalBounds();
    _titleText.setOrigin(titleBounds.width / 2.f, titleBounds.height / 2.f);
    _titleText.setPosition(_game->window().getSize().x / 2.f, 100.f);

    // Set up the tagline text
    _taglineText.setFont(Assets::getInstance().getFont("Arcade"));
    _taglineText.setString("Bark, Dash, and Race to Glory!");
    _taglineText.setCharacterSize(24);
    _taglineText.setFillColor(sf::Color(255, 185, 15)); // Golden yellow
    sf::FloatRect taglineBounds = _taglineText.getLocalBounds();
    _taglineText.setOrigin(taglineBounds.width / 2.f, taglineBounds.height / 2.f);
    _taglineText.setPosition(_game->window().getSize().x / 2.f, 160.f);

    // Set up controls panel
    _controlsPanel.setSize(sf::Vector2f(1200.f, 1200.f));
    _controlsPanel.setFillColor(sf::Color(0, 0, 0, 150));
    _controlsPanel.setOutlineColor(sf::Color(255, 255, 255, 50));
    _controlsPanel.setOutlineThickness(2.f);
    sf::FloatRect controlsPanelBounds = _controlsPanel.getLocalBounds();
    _controlsPanel.setOrigin(controlsPanelBounds.width / 2.f, controlsPanelBounds.height / 2.f);
    _controlsPanel.setPosition(_game->window().getSize().x / 2.f, 200.f);

    // Set up controls title
    _controlsTitle.setFont(Assets::getInstance().getFont("Arcade"));
    _controlsTitle.setString("GAME CONTROLS");
    _controlsTitle.setCharacterSize(28);
    _controlsTitle.setFillColor(sf::Color(255, 185, 15)); // Golden yellow
    sf::FloatRect controlsTitleBounds = _controlsTitle.getLocalBounds();
    _controlsTitle.setOrigin(controlsTitleBounds.width / 2.f, controlsTitleBounds.height / 2.f);
    _controlsTitle.setPosition(_game->window().getSize().x / 2.f, 310.f);

    // Set up controls text
    _controlsText.setFont(Assets::getInstance().getFont("Press"));
    _controlsText.setString(
        "Movement:\n"
        "W - Move Up\n"
        "A - Move Left\n"
        "S - Move Down\n"
        "D - Move Right\n\n"
        "\n"
        "Collect bones to accelerate!\n\n"
        "In-Game Commands:\n"
        "E - Bark to destroy barrels\n"
        "Q - Quit race\n"
        "Pass through the paws on the track\n to earn more time as a checkpoint"
    );
    _controlsText.setCharacterSize(30);
    _controlsText.setFillColor(sf::Color::White);
    sf::FloatRect controlsTextBounds = _controlsText.getLocalBounds();
    _controlsText.setOrigin(controlsTextBounds.width / 2.f, controlsTextBounds.height / 2.f);
    _controlsText.setPosition(_game->window().getSize().x / 2.f, 510.f);

    // Set up story text
    _storyText.setFont(Assets::getInstance().getFont("Roboto"));
    _storyText.setString(
        "In the town of Puggington, the annual \"Pug Grand Prix\" has become legendary.\n"
        "Train your pug to navigate obstacles and race for the fastest time!\n"
        "Compete on Puggington Park for the best time!"
    );
    _storyText.setCharacterSize(30);
    _storyText.setFillColor(sf::Color(200, 200, 200));
    sf::FloatRect storyTextBounds = _storyText.getLocalBounds();
    _storyText.setOrigin(storyTextBounds.width / 2.f, storyTextBounds.height / 2.f);
    _storyText.setPosition(_game->window().getSize().x / 2.f, 230.f);

    // Set up the continue text
    _continueText.setFont(Assets::getInstance().getFont("Arcade"));
    _continueText.setString("PRESS SPACE TO START THE GAME");
    _continueText.setCharacterSize(24);
    _continueText.setFillColor(sf::Color(255, 185, 15)); // Golden yellow
    sf::FloatRect continueBounds = _continueText.getLocalBounds();
    _continueText.setOrigin(continueBounds.width / 2.f, continueBounds.height / 2.f);
    _continueText.setPosition(_game->window().getSize().x / 2.f, 760.f);
}

void SplashScreen::update(sf::Time dt)
{
    _entityManager.update();
    _elapsedTime += dt.asSeconds();
    _textPulseTime += dt.asSeconds();

    // Create a pulsing effect for the continue text
    float alpha = 155 + 100 * std::sin(_textPulseTime * _pulseRate * 3.14159f);
    _continueText.setFillColor(sf::Color(255, 185, 15, static_cast<sf::Uint8>(alpha)));

    // Simple bounce animation for the title
    float titleY = 100.f + std::sin(_elapsedTime * 1.5f) * 5.0f;
    _titleText.setPosition(_titleText.getPosition().x, titleY);

    // Only show the continue text after a short delay
    if (_elapsedTime < 1.0f) {
        _continueText.setFillColor(sf::Color::Transparent);
    }

    //// Transition to level 1 directly if ready
    //if (_readyToTransition) {
    //    // Create the Level 1 scene as a shared pointer
    //
    //}
}

void SplashScreen::sRender()
{
    // Draw background
    _game->window().draw(_background);

    // Draw controls panel
    _game->window().draw(_controlsPanel);

    // Draw all text elements
    _game->window().draw(_titleText);
    _game->window().draw(_taglineText);
    _game->window().draw(_controlsTitle);
    _game->window().draw(_controlsText);
    _game->window().draw(_storyText);
    _game->window().draw(_continueText);
}

void SplashScreen::sDoAction(const Command& action)
{
    // Only transition if we've waited at least a second to avoid accidental skipping
    if (_elapsedTime >= 1.0f && action.type() == "START" && action.name() == "CONTINUE") {
        auto level1Scene = std::make_shared<GameProject>(_game, "../level1.txt");
        // Transition to Level 1 scene
        _game->changeScene("PLAY", level1Scene);  // true to end the current scene
    }
}