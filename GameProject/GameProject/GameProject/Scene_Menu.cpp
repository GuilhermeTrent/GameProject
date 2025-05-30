#include "Scene_Menu.h"
#include "GameProject.h"
#include "MusicPlayer.h"
#include "SplashScreen.h"
#include <memory>

void Scene_Menu::onEnd()
{
	_game->window().close();
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}



void Scene_Menu::init()
{


	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
	registerAction(sf::Keyboard::D, "PLAY");
	registerAction(sf::Keyboard::Escape, "QUIT");

	m_title = "PUG GRAND PRIX";
	m_menuStrings.push_back("Level 1");
	m_menuStrings.push_back("Level 2");
	m_menuStrings.push_back("Level 3");

	m_levelPaths.push_back("../level1.txt");
	m_levelPaths.push_back("../level2.txt");
	m_levelPaths.push_back("../level3.txt");
	//m_levelPaths.push_back("../level1.txt");

	m_menuText.setFont(Assets::getInstance().getFont("Arcade"));

	const size_t CHAR_SIZE{ 64 };
	m_menuText.setCharacterSize(CHAR_SIZE);

}

void Scene_Menu::update(sf::Time dt)
{
	_entityManager.update();
}


void Scene_Menu::sRender()
{

	sf::View view = _game->window().getView();
	view.setCenter(_game->window().getSize().x / 2.f, _game->window().getSize().y / 2.f);
	_game->window().setView(view);


	sf::Sprite backgroundSprite;
	backgroundSprite.setTexture(Assets::getInstance().getTexture("MenuBackground"));
	backgroundSprite.setPosition(0, 0);
	backgroundSprite.setScale(
		_game->window().getSize().x / backgroundSprite.getLocalBounds().width,
		_game->window().getSize().y / backgroundSprite.getLocalBounds().height
	);


	_game->window().draw(backgroundSprite);


	static const sf::Color selectedColor(255, 255, 255);
	static const sf::Color normalColor(0, 0, 0);

	// Footer text
	sf::Text footer("Game Controls  \nUP: W    \nDOWN: S  \nLEFT: A     \nRIGHT: D    \nQUIT: Q    \nBARK: E",
		Assets::getInstance().getFont("Arial"), 20);
	footer.setFillColor(normalColor);
	footer.setPosition(32, 700);

	// Menu title
	m_menuText.setFillColor(normalColor);
	m_menuText.setString(m_title);
	m_menuText.setPosition(10, 10);
	_game->window().draw(m_menuText);

	// Render menu options
	for (size_t i{ 0 }; i < m_menuStrings.size(); ++i)
	{
		m_menuText.setFillColor((i == m_menuIndex ? selectedColor : normalColor));
		m_menuText.setPosition(32, 32 + (i + 1) * 96);
		m_menuText.setString(m_menuStrings.at(i));
		_game->window().draw(m_menuText);
	}

	// Draw footer
	_game->window().draw(footer);;

}


void Scene_Menu::sDoAction(const Command& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "UP")
		{
			m_menuIndex = (m_menuIndex + m_menuStrings.size() - 1) % m_menuStrings.size();
		}
		else if (action.name() == "DOWN")
		{
			m_menuIndex = (m_menuIndex + 1) % m_menuStrings.size();
		}
		else if (action.name() == "PLAY")
		{
			_game->changeScene("PLAY", std::make_shared<GameProject>(_game, m_levelPaths[m_menuIndex]));
		}
		/*else if (action.name() == "SPLASH")
		{
			_game->changeScene("SPLASH", std::make_shared<SplashScreen>(_game));
		}*/
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
	}

}
