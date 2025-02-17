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
#include <iomanip>

std::mt19937 rng{ std::random_device{}() };

GameProject::GameProject(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, _worldView(gameEngine->window().getDefaultView())

{
	init(levelPath);
}

void GameProject::sAnimation(sf::Time dt)
{
	//for (auto e : _entityManager.getEntities()) {
	//	// update all animations
	//	if (e->getComponent<CAnimation>().has) {
	//		auto& anim = e->getComponent<CAnimation>();
	//		anim.animation.update(dt);

	//		if (anim.animation.hasEnded()) { // for explosion
	//			e->destroy();
	//		}
	//	}
	//}
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

	// Update bark counter text
	_barkText.setString("Barks: " + std::to_string(_barkCounter));

	// Keep text in the top-left even if the camera moves
	_barkText.setPosition(
		_worldView.getCenter().x - _worldView.getSize().x / 2.f + 10.f,
		_worldView.getCenter().y - _worldView.getSize().y / 2.f + 10.f
	);

	sMovement(dt);
	adjustPlayerPosition();
	if (m_countdownTime > 0.0f)
	{
		m_countdownTime -= dt.asSeconds();
		if (m_countdownTime < 0.0f)
		{
			m_countdownTime = 0.0f;
			m_timerActive = true; 
		}
		m_countdownText.setString(std::to_string(static_cast<int>(std::ceil(m_countdownTime)))); // Update text
	}
	else if (m_timerActive)
	{
		m_raceTime += dt.asSeconds();
		m_countdownText.setString(""); 
	}
	annimatePlayer();
	
	spawnBarrel();

	spawnBone();


	if (!_player) return; // Ensure _player exists

	auto& playerTransform = _player->getComponent<CTransform>();
	auto& playerSprite = _player->getComponent<CSprite>();

	for (auto& bone : _bones)
	{
		if (!bone->isActive()) continue;

		auto& boneTransform = bone->getComponent<CTransform>();

		float distance = std::hypot(playerTransform.pos.x - boneTransform.pos.x,
			playerTransform.pos.y - boneTransform.pos.y);

		if (distance < 50.0f) // Pickup range threshold
		{
			bone->destroy(); // Remove the bone
			SoundPlayer::getInstance().play("Fart"); // Play fart sound

			// Boost player speed for 2 seconds
			_playerSpeedBoost = true;
			_speedBoostTimer = 2.0f;
			playerTransform.vel.x += (playerTransform.vel.x >= 0) ? 100.0f : -100.0f;
			playerTransform.vel.y += (playerTransform.vel.y >= 0) ? 50.0f : -50.0f;

			// Change texture if moving to the right
			if (playerTransform.vel.x > 0)
			{
				playerSprite.sprite.setTexture(Assets::getInstance().getTexture("PR_Fart"));
			}
		}
	}

	// Handle speed boost duration
	if (_playerSpeedBoost)
	{
		_speedBoostTimer -= _deltaTime;
		if (_speedBoostTimer <= 0.0f)
		{
			_playerSpeedBoost = false;
			playerTransform.vel = sf::Vector2f(0.0f, 0.0f); // Reset speed

			// Reset texture to default after farting ends
			playerSprite.sprite.setTexture(Assets::getInstance().getTexture("PugRight"));
		}
	}

	
	//handleBarking();

}

//void Scene_Planes::onBark()
//{
//    if (!canBark()) {
//        std::cout << "No more barks left!" << std::endl;
//        return;
//    }
//
//    _barkCount--; // Decrease bark count
//
//    // Update text immediately so it refreshes before the next frame
//    _barkText.setString("Barks: " + std::to_string(_barkCount));
//
//    std::cout << "Barked! Remaining: " << _barkCount << std::endl;
//}

void GameProject::onEnd()
{
}

void GameProject::sSpawnEnemies()
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

	//Barking
	registerAction(sf::Keyboard::E, "BARK");
}

void GameProject::spawnPlayer(sf::Vector2f pos)
{
	_player = _entityManager.addEntity("player");
	_player->addComponent<CTransform>(pos);

	auto& sr = Assets::getInstance().getSpriteRec("playerPug");
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
	

	auto& playerSprite = _player->getComponent<CSprite>().sprite;
	auto& playerState = _player->getComponent<CState>().state;
	auto playerVel = _player->getComponent<CTransform>().vel;

	if (playerVel.x < -0.1f)
	{
		playerState = "left";
		auto& sr = Assets::getInstance().getSpriteRec("PugLeft");
		playerSprite.setTexture(Assets::getInstance().getTexture(sr.texName));
		playerSprite.setTextureRect(sr.texRect);
	}
	else if (playerVel.x > 0.1f)
	{
		playerState = "right";
		auto& sr = Assets::getInstance().getSpriteRec("PugRight");
		playerSprite.setTexture(Assets::getInstance().getTexture(sr.texName));
		playerSprite.setTextureRect(sr.texRect);
	}
	else if (playerVel.y < -0.1f)
	{
		playerState = "up";
		auto& sr = Assets::getInstance().getSpriteRec("PugUp");
		playerSprite.setTexture(Assets::getInstance().getTexture(sr.texName));
		playerSprite.setTextureRect(sr.texRect);
	}
	else if (playerVel.y > 0.1f)
	{
		playerState = "down";
		auto& sr = Assets::getInstance().getSpriteRec("PugDown");
		playerSprite.setTexture(Assets::getInstance().getTexture(sr.texName));
		playerSprite.setTextureRect(sr.texRect);
	}

	
	
}

void GameProject::spawnBarrel()
{

	if (_barrelsSpawned) return;

	_barrels.clear();

	std::uniform_real_distribution<float> distX(0.0f, 1024.0f); // Adjust to match screen width
	std::uniform_real_distribution<float> distY(0.0f, 768.0f);

	for (int i = 0; i < 3; ++i)
	{
		auto barrel = _entityManager.addEntity("Barrel");
		float x = distX(rng);
		float y = distY(rng);
		barrel->addComponent<CTransform>(sf::Vector2f(x, y));
		barrel->addComponent<CSprite>(Assets::getInstance().getTexture("Barrel"));
		_barrels.push_back(barrel);
	}
	_barrelsSpawned = true;
}

void GameProject::spawnBone()
{
	if (_bonesSpawned) return;

	_bones.clear();

	std::uniform_real_distribution<float> distX(0.0f, 1024.0f); // Adjust to match screen width
	std::uniform_real_distribution<float> distY(0.0f, 768.0f);  // Adjust for height

	for (int i = 0; i < 5; ++i)
	{
		auto bone = _entityManager.addEntity("Bone");
		float x = distX(rng);
		float y = distY(rng);
		bone->addComponent<CTransform>(sf::Vector2f(x, y));
		bone->addComponent<CSprite>(Assets::getInstance().getTexture("Bone"));
		_bones.push_back(bone);
	}
	_bonesSpawned = true;
}




void GameProject::handleBarking()
{
	if (!_barrels.empty())
	{
		// Remove the last spawned barrel
		auto barrelToRemove = _barrels.back();
		barrelToRemove->destroy(); // Mark entity for removal
		_barrels.pop_back(); // Remove from the vector

		// Play explosion sound
		SoundPlayer::getInstance().play("Explosion1");

		//_barrels.front()->destroy();
		//_barrels.erase(_barrels.begin()); // Remove from vector
	}
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

	

	//if (!_font.loadFromFile("assets/fonts/Arial.ttf")) { // Replace with your font path
	//	throw std::runtime_error("Failed to load font!");
	//}

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(5);

	_barkText.setFont(Assets::getInstance().getFont("main"));
	_barkText.setPosition(15.f, 25.f);
	_barkText.setCharacterSize(15);

	m_countdownTime = 3.0f;
	m_raceTime = 0.0f;
	m_timerActive = false;


	
	



	//m_game->soundPlayer().play("countdown");
	// Load the font for the bark counter
	/*if (!_barkFont.loadFromFile("assets/fonts/Arcade.ttf")) {
		std::cerr << "Error loading font for bark counter!" << std::endl;
	}*/

	// Set up the bark counter text
	//_barkText.setFont(_barkFont);
	//_barkText.setCharacterSize(24);
	//_barkText.setFillColor(sf::Color::White);
	//_barkText.setStyle(sf::Text::Bold);
	//_barkText.setString("Barks: " + std::to_string(_barkCounter)); // Start with 2 barks
	//_barkText.setPosition(10.f, 10.f);

	//_barkText.setFont(_font);
	//_barkText.setCharacterSize(24);
	//_barkText.setFillColor(sf::Color::White);
	//_barkText.setPosition(10.f, 10.f); // Top-left corner
	//updateBarkText();
	
	m_countdownText.setFont(Assets::getInstance().getFont("main"));
	m_countdownText.setCharacterSize(30);
	m_countdownText.setFillColor(sf::Color::Red);
	m_countdownText.setPosition(_worldView.getSize().x / 2.f - 20.f, _worldView.getSize().y / 2.f - 20.f);
	m_countdownText.setString(std::to_string(static_cast<int>(m_countdownTime))); // Initial display

	
	
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
		if (command.name() == "BARK") {
			if (_barkCounter > 0) {
				SoundPlayer::getInstance().play("PugBarks"); 
				_barkCounter-= 1;
				_barkText.setString("Barks: " + std::to_string(_barkCounter));
				//_barkCounter--;                               // Decrease the counter
				//updateBarkText();                             // Update displayed text
			}
			handleBarking();


		}
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

//

//void GameProject::spawnBarrels() {
//	
//	std::uniform_real_distribution<float> xPos(50, 750);
//	std::uniform_real_distribution<float> yPos(50, 550);
//
//	for (int i = 0; i < 4; i++) {
//		auto barrel = _entityManager.addEntity("Barrel");
//		barrel->addComponent<CTransform>(sf::Vector2f(xPos(rng), yPos(rng)));
//		auto anim = Assets::getInstance().getAnimation("barrel");
//		barrel->addComponent<CAnimation>(anim);
//		barrel->addComponent<CBoundingBox>(sf::IntRect(0, 0, anim._frames[0].width, anim._frames[0].height));
//		if (!anim._frames.empty()) {
//			barrel->addComponent<CBoundingBox>(sf::IntRect(0, 0, anim._frames[0].width, anim._frames[0].height));
//		}
//	}
//	
//}
//
//void GameProject::checkBarkCollision() {
//	for (auto barrel : _entityManager.getEntities("Barrel")) {
//		auto overlap = Physics::getOverlap(_player, barrel);
//		if (overlap.x > 0 && overlap.y > 0) {
//			startAnimation(barrel, "explosion");
//			barrel->destroy();
//			SoundPlayer::getInstance().play("Explosion1", barrel->getComponent<CTransform>().pos);
//		}
//	}
//}

//void GameProject::startAnimation(sPtrEntt e, std::string animation) {
//	e->addComponent<CAnimation>(Assets::getInstance().getAnimation(animation));
//	e->getComponent<CTransform>().vel = sf::Vector2f(0, 0);
//	e->removeComponent<CBoundingBox>();
//	e->addComponent<CState>().state = "Exploding";
//}

//void GameProject::spawnBarrels() {
//	std::random_device rd;
//	std::mt19937 gen(rd());
//	std::uniform_real_distribution<float> xDist(100.f, 700.f);
//	std::uniform_real_distribution<float> yDist(100.f, 500.f);
//
//	for (int i = 0; i < 5; ++i) {
//		auto barrel = m_game->_entityManager.addEntity("barrel");
//		barrel->setPosition(sf::Vector2f(xDist(gen), yDist(gen)));
//	}
//}
//
//
//void GameProject::checkBarkCollision() {
//	auto& barrels = _entityManager.getEntities("barrel"); // Use correct way to get entities
//
//	for (auto& barrel : barrels) {
//		if (!barrel->isActive()) continue;
//
//		// Assuming position is stored in a component like CTransform
//		if (barrel->hasComponent<CTransform>()) {
//			auto barrelPos = barrel->getComponent<CTransform>()->pos;
//			auto playerPos = _player->getComponent<CTransform>()->pos;
//
//			float barkRange = 100.0f; // Adjust as needed
//			if (distance(playerPos, barrelPos) < barkRange) {
//				barrel->destroy(); // Assuming this method exists
//			}
//		}
//	}
//}
//
//void GameProject::onBark() {
//	if (!canBark()) return;
//
//	_barkCounter--;
//	updateBarkText();
//
//	checkBarkCollision();
//}

//void GameProject::spawnBarrel()
//{
//	const int numBarrels = 3; // Number of barrels to spawn
//	std::vector<sf::Vector2f> barrelPositions;
//
//	// Define the boundaries for spawning barrels
//	float minX = 50.0f, maxX = 750.0f; // Adjust according to your map size
//	float minY = 100.0f, maxY = 500.0f;
//
//	for (int i = 0; i < numBarrels; ++i)
//	{
//		// Generate a random position within the defined bounds
//		float x = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
//		float y = minY + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxY - minY)));
//
//		sf::Vector2f position(x, y);
//		barrelPositions.push_back(position);
//
//		// Create the barrel entity
//		auto entity = _entityManager.addEntity("barrel");
//
//		// Assign components
//		entity->addComponent<CTransform>(position);
//		entity->addComponent<CBoundingBox>(sf::Vector2f(32, 32)); // Assuming 32x32 sprite size
//		//entity->addComponent<CAnimation>(m_game->assets().getAnimation("Barrel"), true);
//
//		// Add any other necessary components (e.g., physics, interactions)
//	}
//}




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
		_barkText.setString("Barks: " + std::to_string(_barkCounter));
		_barkText.setPosition(
			_game->window().getSize().x - _barkText.getLocalBounds().width - 10,
			10
		);
		_game->window().draw(_barkText);

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

		//; >assets().getFont("Arcade");
		sf::Text timerText;
		timerText.setFont(Assets::getInstance().getFont("main"));
		timerText.setCharacterSize(50);
		timerText.setFillColor(sf::Color::White);
		timerText.setPosition(_game->window().getSize().x / 2 - 50, 50);
		// Format time as MM:SS.mmm (minutes, seconds, milliseconds)
		int minutes = static_cast<int>(m_raceTime) / 60;
		int seconds = static_cast<int>(m_raceTime) % 60;
		int milliseconds = static_cast<int>((m_raceTime - static_cast<int>(m_raceTime)) * 1000);

		
		std::ostringstream timeStream;
		timeStream << std::setfill('0') << std::setw(2) << minutes << ":"
			<< std::setw(2) << seconds << "."
			<< std::setw(3) << milliseconds;

		

		if (m_countdownTime > 0.0f)
		{
			timerText.setString(std::to_string(static_cast<int>(ceil(m_countdownTime))));
		}
		else
		{
			timerText.setString("Time: " + std::to_string(static_cast<int>(m_raceTime)));
		}
		timerText.setString(timeStream.str()); 

		_game->window().draw(timerText);
		_game->window().draw(m_countdownText);
	}
	spawnBarrel();
	/*_barkText.setPosition(5.0f, -5.0f);
	_barkText.setString("Barks  " + std::to_string(_barkCounter));
	_game->window().draw(_barkText);*/
	// Draw bark counter
	//_game->window().draw(_barkText);
}
