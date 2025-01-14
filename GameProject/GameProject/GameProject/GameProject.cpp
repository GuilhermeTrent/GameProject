#include "GameProject.h"
#include "Components.h"
#include "Physics.h"
#include "Utilities.h"
#include "MusicPlayer.h"
#include "Assets.h"
#include "SoundPlayer.h"
#include "GameEngine.h"

#include <random>
#include <fstream>
#include <iostream>

GameProject::GameProject(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, _worldView(gameEngine->window().getDefaultView())

{
	init(levelPath);
}

void GameProject::sAnimation(sf::Time dt)
{
}

void GameProject::sMovement(sf::Time dt)
{
	playerMovement();

	// move all objects
	for (auto& e : _entityManager.getEntities()) {
		if (e->hasComponent<CTransform>()) {
			auto& tfm = e->getComponent<CTransform>();
			tfm.pos += tfm.vel * dt.asSeconds();
			tfm.angle += tfm.angVel * dt.asSeconds();
		}
	}
}

void GameProject::sCollisions()
{
}

void GameProject::sUpdate(sf::Time dt)
{
	if (_isPaused)
		return;

	SoundPlayer::getInstance().removeStoppedSounds();
	_entityManager.update();

	sMovement(dt);
	adjustPlayerPosition();

}

void GameProject::onEnd()
{
}

void GameProject::sSpawnEnemies()
{
}

void GameProject::startAnimation(sPtrEntt e, std::string animation)
{
}

void GameProject::checkIfDead(sPtrEntt e)
{
}

void GameProject::checkPlayerCollision()
{
}

void GameProject::destroyOutsideWindow()
{
}

void GameProject::spawnEnemy(SpawnPoint sp)
{
}

void GameProject::registerActions()
{
	registerAction(sf::Keyboard::Z, "ZOOMOUT");
	registerAction(sf::Keyboard::X, "ZOOMIN");


	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "BACK");
	registerAction(sf::Keyboard::Q, "QUIT");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");

	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::Left, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Right, "RIGHT");
	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::Up, "UP");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::Down, "DOWN");
}

void GameProject::spawnPlayer(sf::Vector2f pos)
{
	_player = _entityManager.addEntity("player");
	_player->addComponent<CTransform>(pos);

	auto& sr = Assets::getInstance().getSpriteRec("playerFrog");
	auto& sprite = _player->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
	sprite.setTextureRect(sr.texRect);
	centerOrigin(sprite);

	_player->addComponent<CBoundingBox>(sf::Vector2f{ 35.f,33.f });
	_player->addComponent<CState>("straight");
	_player->addComponent<CInput>();
}

void GameProject::playerMovement()
{
	sf::Vector2f playerVel{ 0.f, 0.f };
	auto& pInput = _player->getComponent<CInput>();
	if (pInput.left) playerVel.x -= 1;
	if (pInput.right) playerVel.x += 1;
	if (pInput.up) playerVel.y -= 1;
	if (pInput.down) playerVel.y += 1;

	playerVel = _config.playerSpeed * normalize(playerVel);
	_player->getComponent<CTransform>().vel = playerVel;
}

void GameProject::annimatePlayer()
{
}

void GameProject::adjustPlayerPosition()
{
	// don't ajust position if dead
	if (_player->getComponent<CState>().state == "dead")
		return;

	auto center = _worldView.getCenter();
	sf::Vector2f viewHalfSize = _worldView.getSize() / 2.f;

	auto left = center.x - viewHalfSize.x;
	auto right = center.x + viewHalfSize.x;
	auto top = center.y - viewHalfSize.y;
	auto bot = center.y + viewHalfSize.y;

	auto& player_pos = _player->getComponent<CTransform>().pos;
	auto halfSize = _player->getComponent<CBoundingBox>().halfSize;

	// keep player in bounds
	player_pos.x = std::max(player_pos.x, left + halfSize.x);
	player_pos.x = std::min(player_pos.x, right - halfSize.x);
	player_pos.y = std::max(player_pos.y, top + halfSize.y);
	player_pos.y = std::min(player_pos.y, bot - halfSize.y);
}

void GameProject::init(const std::string& levelPath)
{
	loadLevel(levelPath);
	registerActions();

	sf::Vector2f spawnPos{ _worldView.getSize().x / 2.f, _worldBounds.height - _worldView.getSize().y / 2.f };

	_worldView.setCenter(spawnPos);

	spawnPlayer(spawnPos);

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(5);
}

void GameProject::loadLevel(const std::string& path)
{
	std::ifstream config(path);

	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Bkg") {
			std::string name;
			sf::Vector2f pos;
			config >> name >> pos.x >> pos.y;
			auto e = _entityManager.addEntity("bkg");

			// for background, no textureRect its just the whole texture
			// and no center origin, position by top left corner
			auto& sprite = e->addComponent<CSprite>(Assets::getInstance().getTexture(name)).sprite;
			sprite.setOrigin(0.f, 0.f);
			sprite.setPosition(pos);
		}
		else if (token == "World") {
			config >> _worldBounds.width >> _worldBounds.height;
		}
		else if (token == "PlayerSpeed") {
			config >> _config.playerSpeed;
		}
		config >> token;
	}

	config.close();
}

void GameProject::update(sf::Time dt)
{
	sUpdate(dt);
}

void GameProject::sDoAction(const Command& command)
{
	// On Key Press
	if (command.type() == "START") {
		if (command.name() == "PAUSE") { setPaused(!_isPaused); }
		else if (command.name() == "QUIT") { _game->quitLevel(); }
		else if (command.name() == "BACK") { _game->backLevel(); }

		else if (command.name() == "ZOOMOUT") { _worldView.zoom(1.5); }
		else if (command.name() == "ZOOMIN") { _worldView.zoom(0.66667); }

		else if (command.name() == "TOGGLE_TEXTURE") { _drawTextures = !_drawTextures; }
		else if (command.name() == "TOGGLE_COLLISION") { _drawAABB = !_drawAABB; }

		// Player control
		else if (command.name() == "LEFT") { _player->getComponent<CInput>().left = true; }
		else if (command.name() == "RIGHT") { _player->getComponent<CInput>().right = true; }
		else if (command.name() == "UP") { _player->getComponent<CInput>().up = true; }
		else if (command.name() == "DOWN") { _player->getComponent<CInput>().down = true; }

	}
	// on Key Release
	else if (command.type() == "END") {
		// Player control
		if (command.name() == "LEFT") { _player->getComponent<CInput>().left = false; }
		else if (command.name() == "RIGHT") { _player->getComponent<CInput>().right = false; }
		else if (command.name() == "UP") { _player->getComponent<CInput>().up = false; }
		else if (command.name() == "DOWN") { _player->getComponent<CInput>().down = false; }
	}
}

void GameProject::sRender()
{
	_game->window().setView(_worldView);

	// draw bkg first
	for (auto e : _entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;
			_game->window().draw(sprite);
		}
	}

	for (auto& e : _entityManager.getEntities()) {
		if (!e->hasComponent<CSprite>() || e->getTag() == "bkg")
			continue;

		// Draw Sprite
		auto& sprite = e->getComponent<CSprite>().sprite;
		auto& tfm = e->getComponent<CTransform>();
		sprite.setPosition(tfm.pos);
		sprite.setRotation(tfm.angle);
		_game->window().draw(sprite);

		if (_drawAABB && e->hasComponent<CBoundingBox>()) {
			auto box = e->getComponent<CBoundingBox>();
			sf::RectangleShape rect;
			rect.setSize(sf::Vector2f{ box.size.x, box.size.y });
			centerOrigin(rect);
			rect.setPosition(e->getComponent<CTransform>().pos);
			rect.setFillColor(sf::Color(0, 0, 0, 0));
			rect.setOutlineColor(sf::Color{ 0, 255, 0 });
			rect.setOutlineThickness(2.f);
			_game->window().draw(rect);
		}
	}
}
