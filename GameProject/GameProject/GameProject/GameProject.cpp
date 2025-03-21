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


struct Snowflake {
	sf::CircleShape shape;
	float speed;
};


std::vector<Snowflake> snowflakes;

void initSnowflakes(int count) {
	snowflakes.clear();
	for (int i = 0; i < count; i++) {
		Snowflake snow;
		snow.shape.setRadius(static_cast<float>(rand() % 3 + 2));
		snow.shape.setFillColor(sf::Color(255, 255, 255, 150));
		snow.shape.setPosition(rand() % 1920, rand() % 1080);
		snow.speed = static_cast<float>(rand() % 3 + 1);
		snowflakes.push_back(snow);
	}
}




void updateSnowflakes() {
	for (auto& snow : snowflakes) {
		snow.shape.move(0, snow.speed);
		if (snow.shape.getPosition().y > 1080) {
			snow.shape.setPosition(rand() % 1920, -10);
		}
	}
}


void renderSnowflakes(sf::RenderWindow& window) {
	for (const auto& snow : snowflakes) {
		window.draw(snow.shape);
	}
}

GameProject::GameProject(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine)
	, _worldView(gameEngine->window().getDefaultView()),
	_levelPath(levelPath)

{
	init(levelPath);

	spawnPlayerForLevel();


	if (levelPath.find("../level3.txt") != std::string::npos) {
		_enableSnow = true;
		initSnowflakes(100); 
	}
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

	
	_barkText.setString("Barks: " + std::to_string(_barkCounter));

	
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


	if (!_player) return; 

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

			
			if (playerTransform.vel.x > 0)
			{
				auto& sr = Assets::getInstance().getSpriteRec("PR_Fart");
				playerSprite.sprite.setTexture(Assets::getInstance().getTexture("PR_Fart"));
				/*playerSprite.setTexture(Assets::getInstance().getTexture(sr.texName));
				playerSprite.setTextureRec(sr.texRect);*/
			}
		}
		// Only update snow if it's enabled
		if (_enableSnow) {
			updateSnowflakes();
		}
	}

	// Handle speed boost duration
	if (_playerSpeedBoost)
	{
		_speedBoostTimer -= dt.asSeconds();
		if (_speedBoostTimer <= 0.0f)
		{
			_playerSpeedBoost = false;
			_speedBoostTimer = 0.0f;

			// Reset speed to normal (instead of setting it to zero)
			_player->getComponent<CTransform>().vel = normalize(_player->getComponent<CTransform>().vel) * _config.playerSpeed;
			//_playerSpeedBoost = false;
			//playerTransform.vel = sf::Vector2f(0.0f, 0.0f); // Reset speed

			// Reset texture to default after farting ends
			//playerSprite.sprite.setTexture(Assets::getInstance().getTexture("PugRight"));
			auto& sr = Assets::getInstance().getSpriteRec("PugRight");
			playerSprite.sprite.setTexture(Assets::getInstance().getTexture(sr.texName));
			playerSprite.sprite.setTextureRect(sr.texRect);
		}
	}

	
	//handleBarking();

}



void GameProject::onEnd()
{
}

void GameProject::sSpawnEnemies()
{
}



void GameProject::checkIfDead(sPtrEntt e)
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
	if (!m_timerActive)
	{
		
		_player->getComponent<CTransform>().vel = sf::Vector2f(0.f, 0.f);
		return;
	}
	if (_playerSpeedBoost) return;

	sf::Vector2f playerVel{ 0.f, 0.f };
	auto& pInput = _player->getComponent<CInput>();
	if (pInput.left) playerVel.x -= 1;
	if (pInput.right) playerVel.x += 1;
	if (pInput.up) playerVel.y -= 1;
	if (pInput.down) playerVel.y += 1;

	playerVel = _config.playerSpeed * normalize(playerVel);

	auto& playerPos = _player->getComponent<CTransform>().pos;
	sf::Vector2u imageSize = _backgroundImage.getSize();
	unsigned int px = static_cast<unsigned int>(playerPos.x);
	unsigned int py = static_cast<unsigned int>(playerPos.y);

	
	if (px < imageSize.x && py < imageSize.y) {
		sf::Color pixelColor = _backgroundImage.getPixel(px, py);

		// Range green
		if (pixelColor.r >= 15 && pixelColor.r <= 30 &&  
			pixelColor.g >= 210 && pixelColor.g <= 230 && 
			pixelColor.b >= 20 && pixelColor.b <= 35) {  
			playerVel *= 0.5f; 
		}
	}
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

//bool checkCollision(Player player, Barrel barrel) {
//	return (player.x < barrel.x + barrel.width &&
//		player.x + player.width > barrel.x &&
//		player.y < barrel.y + barrel.height &&
//		player.y + player.height > barrel.y);
//}

void GameProject::spawnBarrel()
{

	if (_barrelsSpawned) return;

	_barrels.clear();
	int mapWidth, mapHeight;
	sf::Image* selectedBackground;

	// Determine which background is being used
	if (_levelPath.find("level2.txt") != std::string::npos) {
		selectedBackground = &_backgroundImageBeach;
	}
	else if (_levelPath.find("level3.txt") != std::string::npos) {
		selectedBackground = &_backgroundImageSnow;
	}
	else {
		selectedBackground = &_backgroundImage;
	}

	mapWidth = selectedBackground->getSize().x;
	mapHeight = selectedBackground->getSize().y;

	std::uniform_int_distribution<int> distX(0, mapWidth - 1);
	std::uniform_int_distribution<int> distY(0, mapHeight - 1);

	for (int i = 0; i < 3; ++i) {
		bool validSpawn = false;
		int x, y;

		while (!validSpawn) {
			x = distX(rng);
			y = distY(rng);

			if (x < mapWidth && y < mapHeight) {
				sf::Color pixelColor = selectedBackground->getPixel(x, y);

				if ((selectedBackground == &_backgroundImage && pixelColor.r == 66 && pixelColor.g == 80 && pixelColor.b == 86) ||
					(selectedBackground == &_backgroundImageBeach && pixelColor.r == 66 && pixelColor.g == 80 && pixelColor.b == 86) ||
					(selectedBackground == &_backgroundImageSnow && pixelColor.r == 100 && pixelColor.g == 103 && pixelColor.b == 100)) {
					validSpawn = true;
				}
			}
		}

		auto barrel = _entityManager.addEntity("Barrel");
		barrel->addComponent<CTransform>(sf::Vector2f(x, y));
		barrel->addComponent<CSprite>(Assets::getInstance().getTexture("Barrel"));
		_barrels.push_back(barrel);
	}
	_barrelsSpawned = true;

	/*if (_barrelsSpawned) return;
	int mapWidth = _backgroundImage.getSize().x;
	int mapHeight = _backgroundImage.getSize().y;
	

	_barrels.clear();
	std::uniform_int_distribution<int> distX(0, mapWidth - 1);
	std::uniform_int_distribution<int> distY(0, mapHeight - 1);
	



	for (int i = 0; i < 3; ++i) {
		bool validSpawn = false;
		int x, y;

		while (!validSpawn) {
			x = distX(rng);
			y = distY(rng);
			sf::Vector2u imageSize = _backgroundImage.getSize();
			sf::Vector2u imageSizeB = _backgroundImageBeach.getSize();
			sf::Vector2u imageSizeS = _backgroundImageSnow.getSize();
			if (x < imageSize.x && y < imageSize.y) {
				sf::Color pixelColor = _backgroundImage.getPixel(x, y);

				if (pixelColor.r == 66 && pixelColor.g == 80 && pixelColor.b == 86) {
					validSpawn = true;
				}
			}
			else if (x < imageSizeB.x && y < imageSizeB.y) {
				sf::Color pixelColor = _backgroundImageBeach.getPixel(x, y);
				if (pixelColor.r == 66 && pixelColor.g == 80 && pixelColor.b == 86) {
					validSpawn = true;
				}
			}
			else if (x < imageSizeS.x && y < imageSizeS.y) {
				sf::Color pixelColor = _backgroundImageSnow.getPixel(x, y);
				if (pixelColor.r == 100 && pixelColor.g == 103 && pixelColor.b == 100) {
					validSpawn = true;
				}
			}
		}

		auto barrel = _entityManager.addEntity("Barrel");
		barrel->addComponent<CTransform>(sf::Vector2f(x, y));
		barrel->addComponent<CSprite>(Assets::getInstance().getTexture("Barrel"));
		_barrels.push_back(barrel);
	}
	_barrelsSpawned = true;*/
	
	
	/*if (_barrelsSpawned) return;

	_barrels.clear();

	std::uniform_real_distribution<float> distX(0.0f, 1920.0f); 
	std::uniform_real_distribution<float> distY(0.0f, 1080.0f);

	for (int i = 0; i < 3; ++i)
	{
		auto barrel = _entityManager.addEntity("Barrel");
		float x = distX(rng);
		float y = distY(rng);
		barrel->addComponent<CTransform>(sf::Vector2f(x, y));
		barrel->addComponent<CSprite>(Assets::getInstance().getTexture("Barrel"));
		_barrels.push_back(barrel);
	}
	_barrelsSpawned = true;*/
}

void GameProject::spawnBone()
{
	if (_bonesSpawned) return;

	_bones.clear();

	std::uniform_real_distribution<float> distX(0.0f, 1920.0f); 
	std::uniform_real_distribution<float> distY(0.0f, 1080.0f);  

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
		
		auto barrelToRemove = _barrels.back();
		barrelToRemove->destroy(); 
		_barrels.pop_back(); 

		
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
	_backgroundImage = Assets::getInstance().getTexture("Park").copyToImage();
	_backgroundImageBeach = Assets::getInstance().getTexture("Beach").copyToImage();
	_backgroundImageSnow = Assets::getInstance().getTexture("Snow").copyToImage();
	loadLevel(levelPath);
	registerActions();

	sf::Vector2f spawnPos{ _worldView.getSize().x / 2.f, _worldBounds.height - _worldView.getSize().y / 2.f };

	_worldView.setCenter(spawnPos);

	spawnPlayer(spawnPos);

	
	initSnowflakes(100);
	

	MusicPlayer::getInstance().play("gameTheme");
	MusicPlayer::getInstance().setVolume(5);

	_barkText.setFont(Assets::getInstance().getFont("main"));
	_barkText.setPosition(15.f, 25.f);
	_barkText.setCharacterSize(15);

	m_countdownTime = 3.0f;
	m_raceTime = 0.0f;
	m_timerActive = false;


	
	



	
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
				handleBarking();
			}
			


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
void GameProject::spawnPlayerForLevel()
{
	sf::Vector2f startPosition;

	if (_levelPath.find("level1.txt") != std::string::npos) {
		startPosition = sf::Vector2f(200.f, 500.f); // Adjust as needed for level 1
	}
	else if (_levelPath.find("level2.txt") != std::string::npos) {
		startPosition = sf::Vector2f(300.f, 600.f); // Adjust as needed for level 2
	}
	else if (_levelPath.find("level3.txt") != std::string::npos) {
		startPosition = sf::Vector2f(400.f, 700.f); // Adjust as needed for level 3
	}
	else {
		startPosition = sf::Vector2f(100.f, 500.f); // Default position if level is unknown
	}

	spawnPlayer(startPosition);
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
	
	if (_enableSnow) {
		renderSnowflakes(_game->window());
	}
}
