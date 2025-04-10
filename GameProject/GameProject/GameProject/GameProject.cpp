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


std::vector<PlayerRecord> _playerRecords;
int _currentPlayer = 1;
float _switchPlayerCountdown = 3.0f; // Time before switching to next player
bool _showSwitchMessage = false;
bool _showGameOverMessage = false;
float _gameOverMessageDuration = 3.0f;
std::string _gameOverReason;
bool _showWinnerMessage = false;
float _winnerMessageDuration = 5.0f;
std::string _winnerMessage;


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

std::vector<BlockingSquare> obstacles;
void GameProject::generateBlockingSquares()
{
	if (_backgroundImage.getSize().x == 0 || _backgroundImage.getSize().y == 0)
		return;

	std::cout << "Generating Blocking Squares...\n";

	obstacles.clear();
	unsigned int width = _backgroundImage.getSize().x;
	unsigned int height = _backgroundImage.getSize().y;


	const int STEP_SIZE = 40;
	const int BLOCK_SIZE = 40; // Original block size
	const int COLLISION_SIZE = 16; // Smaller collision size
	const int BOUNDARY_OFFSET = 100; // Increased from 50 to 120 - allows more space between track and obstacles
	const int GRASS_MARGIN = 120; // How much grass area to leave accessible


	std::set<std::pair<int, int>> blockPositions;


	for (unsigned int y = 0; y < height; y += STEP_SIZE)
	{
		for (unsigned int x = 0; x < width; x += STEP_SIZE)
		{
			// Check if current pixel is track
			sf::Color pixelColor = _backgroundImage.getPixel(x, y);
			if (pixelColor.r == 66 && pixelColor.g == 80 && pixelColor.b == 86)
			{
				// Scan around the track pixel for grass, but farther away from the track edge
				for (int dy = -BOUNDARY_OFFSET; dy <= BOUNDARY_OFFSET; dy += BLOCK_SIZE)
				{
					for (int dx = -BOUNDARY_OFFSET; dx <= BOUNDARY_OFFSET; dx += BLOCK_SIZE)
					{
						int newX = x + dx;
						int newY = y + dy;

						// Boundary check
						if (newX >= 0 && newX < width && newY >= 0 && newY < height)
						{
							sf::Color grassColor = _backgroundImage.getPixel(newX, newY);

							// Check for grass color
							if (grassColor.r >= 15 && grassColor.r <= 30 &&
								grassColor.g >= 210 && grassColor.g <= 230 &&
								grassColor.b >= 20 && grassColor.b <= 35)
							{
								// Only place blocking squares beyond the GRASS_MARGIN distance
								float distanceToTrack = std::sqrt(dx * dx + dy * dy);
								if (distanceToTrack > GRASS_MARGIN)
								{
									// Use a set to prevent duplicate blocking squares
									blockPositions.insert({ newX, newY });
								}
							}
						}
					}
				}
			}
		}
	}

	// Convert unique block positions to obstacles with smaller collision boxes
	for (const auto& pos : blockPositions)
	{
		// Center the smaller collision box within the visual block
		int collisionX = pos.first + (BLOCK_SIZE - COLLISION_SIZE) / 2;
		int collisionY = pos.second + (BLOCK_SIZE - COLLISION_SIZE) / 2;

		obstacles.push_back({ collisionX, collisionY, COLLISION_SIZE, COLLISION_SIZE });
	}

	std::cout << "Track Pixels Found: " << blockPositions.size() << std::endl;
	std::cout << "Blocking Squares Generated: " << obstacles.size() << std::endl;
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

	/*_finishLine = sf::FloatRect(100.f, 50.f, 200.f, 20.f);
	_checkpoints.push_back({ sf::FloatRect(300.f, 100.f, 50.f, 20.f) });
	_checkpoints.push_back({ sf::FloatRect(500.f, 300.f, 50.f, 20.f) });*/
}


//void GameProject::resetForPlayer2()
//{
//	// Reset barrels
//	_barrelsSpawned = false;
//	for (auto& barrel : _barrels) {
//		if (barrel && barrel->isActive()) {
//			barrel->destroy();
//		}
//	}
//	_barrels.clear();
//
//	// Reset bones
//	_bonesSpawned = false;
//	for (auto& bone : _bones) {
//		if (bone && bone->isActive()) {
//			bone->destroy();
//		}
//	}
//	_bones.clear();
//
//	// Reset bark counter
//	_barkCounter = 0;
//
//	// Reset race timer
//	m_countdownTime = 3.0f;
//	m_timerActive = false;
//	m_raceTime = 50.0f;  // Same as Player 1
//
//	// Reset lap progress
//	resetLapProgress();
//
//	// Reset player state
//	_playerSpeedBoost = false;
//	_speedBoostTimer = 0.0f;
//}


void GameProject::setupCheckpoints(const std::string& levelPath)
{
	_checkpoints.clear();
	_currentCheckpoint = 0;
	_allCheckpointsReached = false;

	// More precise checkpoint placements based on track layout
	// Let's make them more visible for debugging
	_checkpoints = {
		{ sf::FloatRect(310.f, 100.f, 100.f, 100.f) },
		{ sf::FloatRect(1510.f, 200.f, 100.f, 100.f) },
		{ sf::FloatRect(1370.f, 720.f, 100.f, 100.f) },
		{ sf::FloatRect(290.f, 940.f, 100.f, 100.f) },
		{ sf::FloatRect(1069.f, 650.f, 100.f, 100.f) }


	};

	// Make finish line more visible
	_finishLine = sf::FloatRect(610.f, 400.f, 75.f, 100.f);

	// For debugging, make checkpoints more visible
	for (auto& checkpoint : _checkpoints) {
		checkpoint.area.width = 50.f;  // Make larger for debugging
		checkpoint.area.height = 200.f;
	}

	std::cout << "Checkpoints and finish line set up. Total checkpoints: " << _checkpoints.size() << std::endl;
}



void GameProject::resetLapProgress()
{
	for (auto& checkpoint : _checkpoints) {
		checkpoint.reached = false;
	}
	_currentCheckpoint = 0;
	_allCheckpointsReached = false;
	m_raceStarted = false;
	m_countdownTime = 3.0f; // 3-second countdown
	m_timerActive = false;
	m_raceTime = 10.0f; // Reset race time
	_lapCount = 0;
}


// Add a method to determine the winner
void GameProject::determineWinner()
{
	if (_playerRecords.size() < 2) return;

	const auto& player1 = _playerRecords[0];
	const auto& player2 = _playerRecords[1];

	std::stringstream ss;

	if (player1.lapTime > player2.lapTime) {
		ss << "Player 1 Wins!\nTime: " << std::fixed << std::setprecision(2) << player1.lapTime << "s\n"
			<< "Player 2 Time: " << player2.lapTime << "s";
	}
	else if (player2.lapTime > player1.lapTime) {
		ss << "Player 2 Wins!\nTime: " << std::fixed << std::setprecision(2) << player2.lapTime << "s\n"
			<< "Player 1 Time: " << player1.lapTime << "s";
	}
	else {
		ss << "It's a Tie!\nBoth players: " << std::fixed << std::setprecision(2) << player1.lapTime << "s";
	}

	_winnerMessage = ss.str();
	_showWinnerMessage = true;
	_winnerMessageDuration = 5.0f;
}


void GameProject::checkLapProgress()
{
	if (!_player) return;

	// Get player bounds
	sf::FloatRect playerBounds = _player->getComponent<CSprite>().sprite.getGlobalBounds();

	// Only process if race has started (countdown finished)
	if (m_countdownTime <= 0.0f) {
		// Check checkpoints in sequence
		if (_currentCheckpoint < _checkpoints.size() &&
			playerBounds.intersects(_checkpoints[_currentCheckpoint].area)) {
			SoundPlayer::getInstance().play("Checkpoint");
			std::cout << "Checkpoint " << _currentCheckpoint + 1 << " reached!" << std::endl;
			_checkpoints[_currentCheckpoint].reached = true;

			// Add time based on which checkpoint was reached
			if (_currentCheckpoint == 0) { // First checkpoint
				m_raceTime += 10.0f; // Add 10 seconds
				_lastTimeBonus = 10.0f;

				// Show time bonus notification
				_showTimeBonus = true;
				_timeBonusDisplayTime = 2.0f; // Show for 2 seconds
			}
			else if (_currentCheckpoint == 1) { // Second checkpoint
				m_raceTime += 15.0f; // Add 15 seconds
				_lastTimeBonus = 15.0f;

				// Show time bonus notification
				_showTimeBonus = true;
				_timeBonusDisplayTime = 2.0f; // Show for 2 seconds
			}
			else if (_currentCheckpoint == 2) { // Second checkpoint
				m_raceTime += 20.0f; // Add 15 seconds
				_lastTimeBonus = 20.0f;

				// Show time bonus notification
				_showTimeBonus = true;
				_timeBonusDisplayTime = 2.0f; // Show for 2 seconds
			}
			else if (_currentCheckpoint == 3) { // Second checkpoint
				m_raceTime += 15.0f; // Add 15 seconds
				_lastTimeBonus = 15.0f;

				// Show time bonus notification
				_showTimeBonus = true;
				_timeBonusDisplayTime = 2.0f; // Show for 2 seconds
			}
			else if (_currentCheckpoint == 4) { // Second checkpoint
				m_raceTime += 15.0f; // Add 15 seconds
				_lastTimeBonus = 15.0f;

				// Show time bonus notification
				_showTimeBonus = true;
				_timeBonusDisplayTime = 2.0f; // Show for 2 seconds
			}

			// Move to next checkpoint
			_currentCheckpoint++;

			// Check if all checkpoints are reached
			if (_currentCheckpoint >= _checkpoints.size()) {
				_allCheckpointsReached = true;
				std::cout << "All checkpoints reached. Head to finish line!" << std::endl;
			}
		}

		// Check finish line if all checkpoints reached
		if (_allCheckpointsReached && playerBounds.intersects(_finishLine)) {
			std::cout << "Finish line crossed with all checkpoints reached." << std::endl;

			// Record lap time
			_lastLapTime = m_raceTime;
			_lapCount++;

			// Save player record
			_playerRecords.push_back({ _lastLapTime, "Player " + std::to_string(_currentPlayer) });

			// Reset for next lap or end race
			if (_lapCount >= 1) { // Single lap race
				// Race complete
				m_timerActive = false;
				_isPaused = true;
				std::cout << "Race Completed! Final Time: " << _lastLapTime << "s" << std::endl;

				// Check if both players have completed the race
				if (_currentPlayer == 1) {
					// First player finished, prepare for player 2
					_showSwitchMessage = true;
					_switchPlayerCountdown = 5.0f;
					spawnBarrel();
					spawnBone();
					_barkCounter = 2;

				}
				else {
					// Both players finished, show winner
					determineWinner();
				}
			}
			else {
				// Reset for next lap
				_allCheckpointsReached = false;
				_currentCheckpoint = 0;
				for (auto& checkpoint : _checkpoints) {
					checkpoint.reached = false;
				}
				std::cout << "Lap " << _lapCount << " Completed! Time: " << _lastLapTime << "s" << std::endl;
			}
		}
	}

}

void GameProject::resetRaceState()
{
	// Clear all barrels and bones
	for (auto& barrel : _barrels) {
		if (barrel->isActive()) barrel->destroy();
	}
	for (auto& bone : _bones) {
		if (bone->isActive()) bone->destroy();
	}
	_barrels.clear();
	_bones.clear();
	_barrelsSpawned = false;
	_bonesSpawned = false;

	// Reset bark count
	_barkCounter = 2;

	// Reset timer
	m_raceTime = 50.0f;
	m_timerActive = false;

	// Reset speed boost
	_playerSpeedBoost = false;
	_speedBoostTimer = 0.0f;

	// Clear any remaining effects
	if (_player && _player->hasComponent<CSprite>()) {
		auto& playerSprite = _player->getComponent<CSprite>();
		auto& sr = Assets::getInstance().getSpriteRec("PugLeft");
		playerSprite.sprite.setTexture(Assets::getInstance().getTexture(sr.texName));
		playerSprite.sprite.setTextureRect(sr.texRect);
	}
}


// Add these declarations to your GameProject.h file in the private section:
/*
private:
	bool _showTimeBonus = false;
	float _timeBonusDisplayTime = 0.0f;
	float _lastTimeBonus = 0.0f;
*/
// Add these declarations to your class header file:






void GameProject::sAnimation(sf::Time dt)
{

	for (auto e : _entityManager.getEntities()) {
		if (!e->hasComponent<CAnimation>()) continue;

		auto& anim = e->getComponent<CAnimation>();
		anim.countDown -= dt;

		std::cout << "Countdown: " << anim.countDown.asSeconds() << " seconds\n";

		if (anim.countDown.asSeconds() <= 0) {
			anim.currentFrame++;
			anim.countDown = anim.timePerFrame; // Reset timer

			std::cout << "Current Frame: " << anim.currentFrame
				<< " / " << anim.numbFrames << "\n";

			if (anim.currentFrame >= anim.numbFrames) {
				if (anim.isRepeat) {
					anim.currentFrame = 0;
				}
				else {
					e->destroy();
					continue;
				}
			}

			auto& sprite = e->getComponent<CSprite>().sprite;
			sprite.setTextureRect(sf::IntRect(
				anim.currentFrame * anim.frameSize.x, 0,
				anim.frameSize.x, anim.frameSize.y
			));

			std::cout << "Setting TextureRect: ("
				<< anim.currentFrame * anim.frameSize.x << ", 0, "
				<< anim.frameSize.x << ", " << anim.frameSize.y << ")\n";
		}
	}
}

//for (auto e : _entityManager.getEntities()) {
//	if (e->hasComponent<CAnimation>()) {
//		auto& anim = e->getComponent<CAnimation>();
//		anim.countDown -= dt; // Correct way to subtract sf::Time



//		if (anim.countDown <= sf::Time::Zero) {
//			anim.currentFrame++;
//			anim.countDown = anim.timePerFrame;

//			if (anim.currentFrame >= anim.numbFrames) {
//				if (anim.isRepeat) {
//					anim.currentFrame = 0;
//				}
//				else {
//					e->destroy();  // Destroy explosion after animation ends
//					continue;
//				}
//			}

//			auto& sprite = e->getComponent<CSprite>().sprite;
//			sprite.setTextureRect(sf::IntRect(
//				anim.currentFrame * anim.frameSize.x, 0,
//				anim.frameSize.x, anim.frameSize.y
//			));
//		}
//	}
//}



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
	if (!_player) return;

	auto& playerTransform = _player->getComponent<CTransform>();
	auto& playerBox = _player->getComponent<CBoundingBox>();

	// Define player bounding rectangle
	sf::FloatRect playerRect(playerTransform.pos.x - playerBox.halfSize.x,
		playerTransform.pos.y - playerBox.halfSize.y,
		playerBox.size.x, playerBox.size.y);

	// Check collision with barrels
	for (auto& barrel : _entityManager.getEntities("Barrel"))
	{
		auto& barrelTransform = barrel->getComponent<CTransform>();
		auto& barrelBox = barrel->getComponent<CBoundingBox>();

		sf::FloatRect barrelRect(barrelTransform.pos.x - barrelBox.halfSize.x,
			barrelTransform.pos.y - barrelBox.halfSize.y,
			barrelBox.size.x, barrelBox.size.y);

		if (playerRect.intersects(barrelRect))
		{
			// Determine pushback direction
			sf::Vector2f pushback(0.f, 0.f);
			if (playerTransform.pos.x < barrelTransform.pos.x)
				pushback.x = -1;
			else if (playerTransform.pos.x > barrelTransform.pos.x)
				pushback.x = 1;

			if (playerTransform.pos.y < barrelTransform.pos.y)
				pushback.y = -1;
			else if (playerTransform.pos.y > barrelTransform.pos.y)
				pushback.y = 1;

			// Move the player back
			playerTransform.pos += pushback * 5.0f; // Adjust 5.0f as needed
		}
	}

	// Check collision with blocking squares
	for (const auto& obstacle : obstacles)
	{
		sf::FloatRect obstacleRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height);

		if (playerRect.intersects(obstacleRect))
		{
			// Store previous position to reset after collision
			sf::Vector2f previousPos = playerTransform.pos;

			// Determine collision side and pushback direction
			sf::Vector2f pushback(0.f, 0.f);


			float overlapX = std::min(playerRect.left + playerRect.width, obstacleRect.left + obstacleRect.width) -
				std::max(playerRect.left, obstacleRect.left);
			float overlapY = std::min(playerRect.top + playerRect.height, obstacleRect.top + obstacleRect.height) -
				std::max(playerRect.top, obstacleRect.top);

			// Push back in the direction of least resistance
			if (overlapX < overlapY) {

				if (playerTransform.pos.x < obstacle.x + obstacle.width / 2)
					pushback.x = -overlapX;
				else
					pushback.x = overlapX;
			}
			else {

				if (playerTransform.pos.y < obstacle.y + obstacle.height / 2)
					pushback.y = -overlapY;
				else
					pushback.y = overlapY;
			}

			// Apply pushback
			playerTransform.pos += pushback;


			sf::FloatRect newPlayerRect(playerTransform.pos.x - playerBox.halfSize.x,
				playerTransform.pos.y - playerBox.halfSize.y,
				playerBox.size.x, playerBox.size.y);

			if (newPlayerRect.intersects(obstacleRect)) {
				playerTransform.pos = previousPos;
			}
		}
	}
}


bool wallsCreated = false;

void GameProject::sUpdate(sf::Time dt)
{
	if (_isPaused) {
		// Handle player switching while paused
		if (_showSwitchMessage) {
			_switchPlayerCountdown -= dt.asSeconds();
			if (_switchPlayerCountdown <= 0.0f) {
				_showSwitchMessage = false;
				_currentPlayer = 2;
				resetLapProgress();
				resetRaceState();
				m_countdownTime = 3.0f; // Reset countdown
				m_timerActive = false;
				_isPaused = false;
			}
		}

		// Handle winner message display
		if (_showWinnerMessage) {
			_winnerMessageDuration -= dt.asSeconds();
			if (_winnerMessageDuration <= 0.0f) {
				_showWinnerMessage = false;
				// Could transition to menu or next level here
			}
		}




		// Handle game over message display
		if (_showGameOverMessage) {
			_gameOverMessageDuration -= dt.asSeconds();
			if (_gameOverMessageDuration <= 0.0f) {
				_showGameOverMessage = false;
				// Could transition to menu or restart here
			}
		}

		return;
	}

	SoundPlayer::getInstance().removeStoppedSounds();
	_entityManager.update();

	_barkText.setString("Barks: " + std::to_string(_barkCounter));
	_barkText.setCharacterSize(30);

	_barkText.setPosition(
		_worldView.getCenter().x - _worldView.getSize().x / 2.f + 10.f,
		_worldView.getCenter().y - _worldView.getSize().y / 2.f + 10.f
	);

	sMovement(dt);
	adjustPlayerPosition();
	sAnimation(dt);

	if (m_countdownTime > 0.0f)
	{

		m_countdownTime -= dt.asSeconds();
		if (m_countdownTime <= 0.0f && !m_timerActive)
		{
			m_countdownTime = 0.0f;
			m_timerActive = true;
			m_raceTime = 50.0f; // Set initial race time (adjust if needed)
		}

		int countdown = static_cast<int>(std::ceil(m_countdownTime));


		if (countdown == 3)
			m_countdownText.setFillColor(sf::Color::Red);
		else if (countdown == 2)
			m_countdownText.setFillColor(sf::Color::Yellow);
		else if (countdown == 1)
			m_countdownText.setFillColor(sf::Color::Green);

		m_countdownText.setString(std::to_string(countdown));
	}
	else if (m_timerActive)
	{
		m_raceTime -= dt.asSeconds();

		if (m_raceTime <= 0.0f)
		{
			m_raceTime = 0.0f;
			_isPaused = true; // Stop race
			_gameOverReason = "Time's Up!";
			_showGameOverMessage = true;
			_gameOverMessageDuration = 3.0f;
			std::cout << "Time's up! Race Over." << std::endl;
			SoundPlayer::getInstance().play("GameOver");
		}

		m_countdownText.setString("Time: " + std::to_string(static_cast<int>(m_raceTime)));

		m_countdownText.setFillColor(sf::Color::White);
	}


	annimatePlayer();
	spawnBarrel();
	spawnBone();
	sCollisions();


	if (!_player) return;

	auto& playerTransform = _player->getComponent<CTransform>();
	auto& playerSprite = _player->getComponent<CSprite>();

	for (auto& bone : _bones)
	{
		if (!bone->isActive()) continue;

		auto& boneTransform = bone->getComponent<CTransform>();

		float distance = std::hypot(playerTransform.pos.x - boneTransform.pos.x,
			playerTransform.pos.y - boneTransform.pos.y);

		if (distance < 50.0f)
		{
			bone->destroy();
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

			}
		}

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

			_player->getComponent<CTransform>().vel = normalize(_player->getComponent<CTransform>().vel) * _config.playerSpeed;

			auto& sr = Assets::getInstance().getSpriteRec("PugRight");
			playerSprite.sprite.setTexture(Assets::getInstance().getTexture(sr.texName));
			playerSprite.sprite.setTextureRect(sr.texRect);
		}
	}
	checkLapProgress();




	//handleBarking();

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

	auto& sr = Assets::getInstance().getSpriteRec("PugLeft");
	auto& sprite = _player->addComponent<CSprite>(Assets::getInstance().getTexture(sr.texName)).sprite;
	sprite.setTextureRect(sr.texRect);
	centerOrigin(sprite);

	_player->addComponent<CBoundingBox>(sf::Vector2f{ 64.f,64.f });
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
		if (barrel->hasComponent<CSprite>()) {
			auto& sprite = barrel->getComponent<CSprite>();

			sf::Vector2u textureSize = sprite.sprite.getTexture()->getSize();



		}
		barrel->addComponent<CBoundingBox>(sf::Vector2f{ 64.f,64.f });
		barrel->addComponent<CTransform>(sf::Vector2f(x, y));
		barrel->addComponent<CSprite>(Assets::getInstance().getTexture("Barrel"));
		/*barrel->addComponent<CCollision>();*/
		_barrels.push_back(barrel);
	}
	_barrelsSpawned = true;





}

void GameProject::spawnBone()
{
	if (_bonesSpawned) return;

	_bones.clear();
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

	for (int i = 0; i < 5; ++i) {
		bool validSpawn = false;
		int x, y;

		while (!validSpawn) {
			x = distX(rng);
			y = distY(rng);

			if (x < mapWidth && y < mapHeight) {
				sf::Color pixelColor = selectedBackground->getPixel(x, y);

				// Ensure the bone spawns on the track
				if ((selectedBackground == &_backgroundImage && pixelColor.r == 66 && pixelColor.g == 80 && pixelColor.b == 86) ||
					(selectedBackground == &_backgroundImageBeach && pixelColor.r == 66 && pixelColor.g == 80 && pixelColor.b == 86) ||
					(selectedBackground == &_backgroundImageSnow && pixelColor.r == 100 && pixelColor.g == 103 && pixelColor.b == 100)) {
					validSpawn = true;
				}
			}
		}

		auto bone = _entityManager.addEntity("Bone");
		bone->addComponent<CTransform>(sf::Vector2f(x, y));
		bone->addComponent<CSprite>(Assets::getInstance().getTexture("Bone"));
		_bones.push_back(bone);
	}

	_bonesSpawned = true;
}

void GameProject::handleBarking()
{
	if (!_player || _barrels.empty()) return;

	// Get player position
	sf::Vector2f playerPos = _player->getComponent<CTransform>().pos;

	// Find the nearest barrel
	std::shared_ptr<Entity> nearestBarrel = nullptr;
	float nearestDistance = std::numeric_limits<float>::max();

	for (auto& barrel : _barrels)
	{
		sf::Vector2f barrelPos = barrel->getComponent<CTransform>().pos;

		// Calculate distance between player and barrel using Pythagorean theorem
		float distance = std::hypot(barrelPos.x - playerPos.x, barrelPos.y - playerPos.y);

		// Update nearest barrel if this is closer
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestBarrel = barrel;
		}
	}

	// Check if player is close enough (within trigger distance) and presses "E"
	float triggerDistance = 150.f;

	if (nearestBarrel && nearestDistance <= triggerDistance)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))  // Check for "E" key press
		{
			sf::Vector2f explosionPos = nearestBarrel->getComponent<CTransform>().pos;

			// Remove the nearest barrel from the list
			auto it = std::find(_barrels.begin(), _barrels.end(), nearestBarrel);
			if (it != _barrels.end())
			{
				_barrels.erase(it);
			}

			// Destroy the barrel entity
			nearestBarrel->destroy();

			// Play random explosion sound
			std::uniform_int_distribution<int> flip(1, 2);
			if (flip(rng) == 1)
				SoundPlayer::getInstance().play("Explosion1", explosionPos);
			else
				SoundPlayer::getInstance().play("Explosion2", explosionPos);

			// Spawn explosion animation
			auto explosion = _entityManager.addEntity("Explosion");
			explosion->addComponent<CTransform>(explosionPos);
			startAnimation(explosion, "Explosion"); // Start animation
		}
	}
}


void GameProject::startAnimation(sPtrEntt e, std::string animation) {
	auto& animComp = e->addComponent<CAnimation>();
	auto& animRec = Assets::getInstance().getAnimationRec(animation);

	animComp.timePerFrame = animRec.duration / static_cast<float>(animRec.numbFrames);
	animComp.frameSize = animRec.frameSize;
	animComp.currentFrame = 0;
	animComp.numbFrames = animRec.numbFrames;
	animComp.countDown = animComp.timePerFrame;
	animComp.isRepeat = animRec.repeat;

	auto& sprite = e->addComponent<CSprite>().sprite;
	sprite.setTexture(Assets::getInstance().getTexture(animRec.texName));
	sprite.setTextureRect(sf::IntRect(0, 0, animRec.frameSize.x, animRec.frameSize.y));
	centerOrigin(sprite);
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
	generateBlockingSquares();
	registerActions();

	initUI();

	sf::Vector2f spawnPos{ _worldView.getSize().x / 2.f, _worldBounds.height - _worldView.getSize().y / 2.f };

	//_worldView.setCenter(spawnPos);

	//spawnPlayer(spawnPos);


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


	setupCheckpoints(levelPath);



	// Start lap timer
	_lapTimer.restart();


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

			/*generateBlockingSquares();*/

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
				_barkCounter -= 1;
				_barkText.setString("Barks: " + std::to_string(_barkCounter));
				handleBarking();
			}



		}
		if (command.name() == "PAUSE") { setPaused(!_isPaused); }
		else if (command.name() == "QUIT") {
			_currentPlayer = 1;
			/*	m_game->setSelectedPlayer(1);*/
			_game->quitLevel();
		}
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





void GameProject::onEnd() {
	// 
}
void GameProject::spawnPlayerForLevel()
{
	sf::Vector2f startPosition;

	if (_levelPath.find("level1.txt") != std::string::npos) {
		startPosition = sf::Vector2f(646.f, 442.f);
	}
	else if (_levelPath.find("level2.txt") != std::string::npos) {
		startPosition = sf::Vector2f(170.f, 706.f);
	}
	else if (_levelPath.find("level3.txt") != std::string::npos) {
		startPosition = sf::Vector2f(184.f, 586.f);
	}
	else {
		startPosition = sf::Vector2f(100.f, 500.f);
	}

	spawnPlayer(startPosition);
}


void GameProject::sRender()
{
	_game->window().setView(_worldView);

	// Draw background first
	for (auto e : _entityManager.getEntities("bkg")) {
		if (e->getComponent<CSprite>().has) {
			auto& sprite = e->getComponent<CSprite>().sprite;
			_game->window().draw(sprite);
		}
	}

	// Draw entities (excluding background)
	for (auto& e : _entityManager.getEntities()) {
		if (!e->hasComponent<CSprite>() || e->getTag() == "bkg")
			continue;

		// Draw Sprite
		auto& sprite = e->getComponent<CSprite>().sprite;
		auto& tfm = e->getComponent<CTransform>();
		sprite.setPosition(tfm.pos);
		sprite.setRotation(tfm.angle);
		_game->window().draw(sprite);

		// Draw bounding boxes if enabled
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

	// Draw obstacles
	for (const auto& obstacle : obstacles)
	{
		// Draw the visual representation (larger)
		sf::RectangleShape visualBlock(sf::Vector2f(10, 10)); // Original BLOCK_SIZE
		visualBlock.setPosition(obstacle.x - (10 - obstacle.width) / 2, obstacle.y - (10 - obstacle.height) / 2);
		visualBlock.setFillColor(sf::Color(0, 0, 0, 0)); // Semi-transparent red
		_game->window().draw(visualBlock);

		// Draw the actual collision box (smaller)
		sf::RectangleShape collisionBlock(sf::Vector2f(obstacle.width, obstacle.height));
		collisionBlock.setPosition(obstacle.x, obstacle.y);
		collisionBlock.setFillColor(sf::Color(0, 0, 0, 0)); // More opaque red
		_game->window().draw(collisionBlock);
	}

	// Additional rendering code (checkpoints, snow, etc.)
	if (_enableSnow) {
		renderSnowflakes(_game->window());
	}

	// Render checkpoints
	sf::RectangleShape checkpointShape;
	checkpointShape.setFillColor(sf::Color(0, 0, 0, 0)); // Transparent red
	//checkpointShape.setOutlineColor(sf::Color::Red);
	//checkpointShape.setOutlineThickness(2.0f);

	for (const auto& checkpoint : _checkpoints)
	{
		checkpointShape.setPosition(checkpoint.area.left, checkpoint.area.top);
		checkpointShape.setSize(sf::Vector2f(checkpoint.area.width, checkpoint.area.height));
		_game->window().draw(checkpointShape);
	}

	// Render finish line
	sf::RectangleShape finishLineShape;
	finishLineShape.setFillColor(sf::Color(0, 0, 0, 0)); // Transparent green
	/*finishLineShape.setOutlineColor(sf::Color::Green);
	finishLineShape.setOutlineThickness(2.0f);*/
	finishLineShape.setPosition(_finishLine.left, _finishLine.top);
	finishLineShape.setSize(sf::Vector2f(_finishLine.width, _finishLine.height));
	_game->window().draw(finishLineShape);

	// --- ENHANCED UI RENDERING ---

	// Format time for display
	int minutes = 0;
	int seconds = 0;
	int milliseconds = 0;
	//MusicPlayer::getInstance().play("Countdown");
	if (m_countdownTime > 0.0f) {


		// During countdown, just show the countdown
		minutes = 0;
		seconds = static_cast<int>(ceil(m_countdownTime));
		milliseconds = 0;
	}
	else {
		// After race starts, show the race time

		minutes = static_cast<int>(m_raceTime) / 60;
		seconds = static_cast<int>(m_raceTime) % 60;
		milliseconds = static_cast<int>((m_raceTime - static_cast<int>(m_raceTime)) * 1000);
	}


	std::ostringstream timeStream;
	timeStream << std::setfill('0') << std::setw(2) << minutes << ":"
		<< std::setw(2) << seconds << "."
		<< std::setw(3) << milliseconds;

	// 1. Timer Panel
	sf::RectangleShape timerPanel(sf::Vector2f(300, 60));
	timerPanel.setFillColor(sf::Color(0, 0, 0, 180)); // Semi-transparent black
	timerPanel.setOutlineColor(sf::Color(255, 255, 255, 100)); // Subtle white outline
	timerPanel.setOutlineThickness(2);
	timerPanel.setPosition((_game->window().getSize().x - timerPanel.getSize().x) - 200, 30); // Top center
	_game->window().draw(timerPanel);

	// Handle countdown vs regular timer display
	_timerText.setCharacterSize(36);
	if (m_countdownTime > 0.0f) {

		// During countdown phase
		_timerText.setString(std::to_string(static_cast<int>(ceil(m_countdownTime))));
		_timerText.setFillColor(sf::Color::Red);
		_timerText.setCharacterSize(72);
	}
	else {
		// Race has started - show time
		_timerText.setString("Time: " + timeStream.str());
		_timerText.setFillColor(sf::Color::White);
	}

	// Center timer text
	sf::FloatRect timerTextBounds = _timerText.getLocalBounds();
	_timerText.setPosition(
		timerPanel.getPosition().x + 20, // Padding from the left of the panel
		timerPanel.getPosition().y + (timerPanel.getSize().y - timerTextBounds.height) / 2 - 8
	);
	_game->window().draw(_timerText);

	// 2. Bark Counter Panel
	sf::RectangleShape barkPanel(sf::Vector2f(150, 50));
	barkPanel.setFillColor(sf::Color(0, 0, 0, 180));
	barkPanel.setOutlineColor(sf::Color(255, 255, 255, 100));
	barkPanel.setOutlineThickness(2);
	barkPanel.setPosition(_game->window().getSize().x - barkPanel.getSize().x - 35, 30); // Top right
	_game->window().draw(barkPanel);

	// Prepare bark text
	_barkText.setString("Barks: " + std::to_string(_barkCounter));

	// Try to position bark icon and text
	if (_barkIconTexture.getSize().x > 0) {
		// If icon exists
		_barkIcon.setPosition(
			barkPanel.getPosition().x + 10,
			barkPanel.getPosition().y + (barkPanel.getSize().y - _barkIcon.getGlobalBounds().height) / 2
		);
		_game->window().draw(_barkIcon);

		_barkText.setString("x " + std::to_string(_barkCounter));
		_barkText.setPosition(
			_barkIcon.getPosition().x + _barkIcon.getGlobalBounds().width + 5,
			barkPanel.getPosition().y + (barkPanel.getSize().y - _barkText.getLocalBounds().height) / 2 - 8
		);
	}
	else {
		// If no icon
		_barkText.setPosition(
			barkPanel.getPosition().x + 20,
			barkPanel.getPosition().y + (barkPanel.getSize().y - _barkText.getLocalBounds().height) / 2 - 8
		);
	}
	_game->window().draw(_barkText);

	// 3. Lap Info Panel (only show if a lap has been completed)
	if (_lapCount > 0) {
		sf::RectangleShape lapPanel(sf::Vector2f(200, 50));
		lapPanel.setFillColor(sf::Color(0, 0, 0, 180));
		lapPanel.setOutlineColor(sf::Color(255, 255, 255, 100));
		lapPanel.setOutlineThickness(2);
		lapPanel.setPosition(10, 10); // Top left
		_game->window().draw(lapPanel);

		// Format lap time
		std::ostringstream lapStream;
		lapStream << "Lap Time: " << std::fixed << std::setprecision(2)
			<< _lastLapTime << "s";
		_lapInfoText.setString(lapStream.str());

		// Position lap text
		sf::FloatRect lapTextBounds = _lapInfoText.getLocalBounds();
		_lapInfoText.setPosition(
			lapPanel.getPosition().x + (lapPanel.getSize().x - lapTextBounds.width) / 2,
			lapPanel.getPosition().y + (lapPanel.getSize().y - lapTextBounds.height) / 2 - 8
		);
		_game->window().draw(_lapInfoText);
	}

	// 4. Big Countdown Display (if active)
	if (m_countdownTime > 0.0f) {
		/*MusicPlayer::getInstance().play("Countdown");*/
		// Background darkening effect
		sf::RectangleShape overlay(sf::Vector2f(_game->window().getSize().x, _game->window().getSize().y));
		overlay.setFillColor(sf::Color(0, 0, 0, 150));
		_game->window().draw(overlay);

		// Big countdown number
		_countdownText.setString(std::to_string(static_cast<int>(ceil(m_countdownTime))));

		// Center the countdown
		sf::FloatRect countdownBounds = _countdownText.getLocalBounds();
		_countdownText.setOrigin(countdownBounds.left + countdownBounds.width / 2.0f,
			countdownBounds.top + countdownBounds.height / 2.0f);
		_countdownText.setPosition(_game->window().getSize().x / 2.0f, _game->window().getSize().y / 2.0f);

		_game->window().draw(_countdownText);
	}




	// Check if countdown just hit zero (transition from positive to zero/negative)
	if (m_prevCountdownTime > 0.0f && m_countdownTime <= 0.0f) {
		m_goDisplayTime = 3.0f; // Set display time to 3 seconds
		std::cout << "GO message triggered! Display time set to " << m_goDisplayTime << std::endl;
	}

	// Save current countdown time for next frame comparison
	m_prevCountdownTime = m_countdownTime;

	// Display the GO message while active
	if (m_goDisplayTime > 0.0f) {
		// Calculate alpha based on fade in/out effect
		float alpha = 255.0f;

		// First second: fade in (3.0 to 2.0 seconds)
		if (m_goDisplayTime > 2.0f) {
			float fadeInProgress = 3.0f - m_goDisplayTime; // 0->1 over first second
			alpha = fadeInProgress * 255.0f;
		}
		// Last second: fade out (1.0 to 0.0 seconds)
		else if (m_goDisplayTime < 1.0f) {
			float fadeOutProgress = m_goDisplayTime; // 1->0 over last second
			alpha = fadeOutProgress * 255.0f;
		}

		// Clamp alpha to valid range
		alpha = std::max(0.0f, std::min(255.0f, alpha));

		// Create and render the GO text
		sf::Text goText;
		goText.setFont(_uiFont);
		goText.setString("GO!");
		goText.setCharacterSize(100);
		goText.setFillColor(sf::Color(0, 255, 0, static_cast<sf::Uint8>(alpha)));
		goText.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
		goText.setOutlineThickness(3.0f);
		sf::FloatRect goBounds = goText.getLocalBounds();
		goText.setOrigin(goBounds.left + goBounds.width / 2.0f,
			goBounds.top + goBounds.height / 2.0f);
		goText.setPosition(_game->window().getSize().x / 2.0f,
			_game->window().getSize().y / 2.0f);
		_game->window().draw(goText);

		// FIXED: Use a minimum delta time to ensure animation progresses
		float dt = _game->deltaTime().asSeconds();
		if (dt <= 0.0001f) {
			dt = 0.016f; // Default to ~60 FPS if delta time is zero
			std::cout << "Warning: Zero delta time detected, using default value" << std::endl;
		}

		// Decrease display time and ensure it stops at zero
		m_goDisplayTime -= dt;
		if (m_goDisplayTime <= 0.0f) {
			m_goDisplayTime = 0.0f;
			std::cout << "GO animation complete" << std::endl;
		}
	}



	// 6. Display time bonus notification when checkpoints are reached
	if (_showTimeBonus) {
		// Calculate alpha based on fade in/out effect
		float alpha = 255.0f;

		// Assuming _timeBonusDisplayTime starts at 3.0f like the GO message
		float totalBonusDisplayTime = 3.0f;

		// First second: fade in (3.0 to 2.0 seconds)
		if (_timeBonusDisplayTime > (totalBonusDisplayTime - 1.0f)) {
			float fadeInProgress = totalBonusDisplayTime - _timeBonusDisplayTime; // 0->1 over first second
			alpha = fadeInProgress * 255.0f;
		}
		// Last second: fade out (1.0 to 0.0 seconds)
		else if (_timeBonusDisplayTime < 1.0f) {
			float fadeOutProgress = _timeBonusDisplayTime; // 1->0 over last second
			alpha = fadeOutProgress * 255.0f;
		}

		// Clamp alpha to valid range
		alpha = std::max(0.0f, std::min(255.0f, alpha));

		sf::Text bonusText;
		bonusText.setFont(_uiFont);
		bonusText.setString("+" + std::to_string(static_cast<int>(_lastTimeBonus)) + "s");
		bonusText.setCharacterSize(48);
		bonusText.setFillColor(sf::Color(255, 255, 0, static_cast<sf::Uint8>(alpha))); // Yellow with alpha
		bonusText.setOutlineColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha))); // Black outline with alpha
		bonusText.setOutlineThickness(2.0f);

		// Position the bonus notification
		sf::FloatRect bonusBounds = bonusText.getLocalBounds();
		bonusText.setOrigin(bonusBounds.left + bonusBounds.width / 2.0f,
			bonusBounds.top + bonusBounds.height / 2.0f);
		bonusText.setPosition(_game->window().getSize().x / 2.0f,
			_game->window().getSize().y / 2.0f + 80.0f); // Below the center

		_game->window().draw(bonusText);

		// Update the bonus notification timer
		float dt = _game->deltaTime().asSeconds();
		if (dt <= 0.0001f) {
			dt = 0.016f; // Default to ~60 FPS if delta time is zero
		}

		_timeBonusDisplayTime -= dt;
		if (_timeBonusDisplayTime <= 0.0f) {
			_timeBonusDisplayTime = 0.0f;
			_showTimeBonus = false;
		}
	}

	if (_showSwitchMessage) {
		// Background darkening effect
		sf::RectangleShape overlay(sf::Vector2f(_game->window().getSize().x, _game->window().getSize().y));
		overlay.setFillColor(sf::Color(0, 0, 0, 180));
		_game->window().draw(overlay);

		// Switch player message
		sf::Text switchText;
		switchText.setFont(_uiFont);
		switchText.setString("Player 1 Finished!\nPlayer 2 starting in " +
			std::to_string(static_cast<int>(ceil(_switchPlayerCountdown))) + "...");
		switchText.setCharacterSize(48);
		switchText.setFillColor(sf::Color::White);
		switchText.setOutlineColor(sf::Color::Black);
		switchText.setOutlineThickness(2.0f);

		// Center the text
		sf::FloatRect switchBounds = switchText.getLocalBounds();
		switchText.setOrigin(switchBounds.left + switchBounds.width / 2.0f,
			switchBounds.top + switchBounds.height / 2.0f);
		switchText.setPosition(_game->window().getSize().x / 2.0f, _game->window().getSize().y / 2.0f);

		_game->window().draw(switchText);
	}

	// Add rendering for game over message
	if (_showGameOverMessage) {
		// Background darkening effect
		sf::RectangleShape overlay(sf::Vector2f(_game->window().getSize().x, _game->window().getSize().y));
		overlay.setFillColor(sf::Color(0, 0, 0, 180));
		_game->window().draw(overlay);

		// Game over message
		sf::Text gameOverText;
		gameOverText.setFont(_uiFont);
		gameOverText.setString(_gameOverReason);
		gameOverText.setCharacterSize(72);
		gameOverText.setFillColor(sf::Color::Red);
		gameOverText.setOutlineColor(sf::Color::White);
		gameOverText.setOutlineThickness(3.0f);

		// Center the text
		sf::FloatRect gameOverBounds = gameOverText.getLocalBounds();
		gameOverText.setOrigin(gameOverBounds.left + gameOverBounds.width / 2.0f,
			gameOverBounds.top + gameOverBounds.height / 2.0f);
		gameOverText.setPosition(_game->window().getSize().x / 2.0f, _game->window().getSize().y / 2.0f);

		_game->window().draw(gameOverText);
	}

	// Add rendering for winner message
	if (_showWinnerMessage) {
		// Background darkening effect
		sf::RectangleShape overlay(sf::Vector2f(_game->window().getSize().x, _game->window().getSize().y));
		overlay.setFillColor(sf::Color(0, 0, 0, 180));
		_game->window().draw(overlay);

		// Winner message
		sf::Text winnerText;
		winnerText.setFont(_uiFont);
		winnerText.setString(_winnerMessage);
		winnerText.setCharacterSize(48);
		winnerText.setFillColor(sf::Color::Yellow);
		winnerText.setOutlineColor(sf::Color::Black);
		winnerText.setOutlineThickness(2.0f);

		// Center the text
		sf::FloatRect winnerBounds = winnerText.getLocalBounds();
		winnerText.setOrigin(winnerBounds.left + winnerBounds.width / 2.0f,
			winnerBounds.top + winnerBounds.height / 2.0f);
		winnerText.setPosition(_game->window().getSize().x / 2.0f, _game->window().getSize().y / 2.0f);

		_game->window().draw(winnerText);
	}
	// Create player panel in the top left
	sf::RectangleShape playerPanel(sf::Vector2f(150, 50));
	playerPanel.setFillColor(sf::Color(0, 0, 0, 180));          // Semi-transparent black
	playerPanel.setOutlineColor(sf::Color(255, 255, 255, 100)); // Semi-transparent white outline
	playerPanel.setOutlineThickness(2);
	playerPanel.setPosition(35, 70); // Top left with the same margins (35px from left, 60px from top)
	_game->window().draw(playerPanel);

	// Display player indicator inside the panel
	sf::Text playerText;
	playerText.setFont(_uiFont);
	playerText.setString("Player " + std::to_string(_currentPlayer));
	playerText.setCharacterSize(24); // Slightly smaller to fit nicely in the panel
	playerText.setFillColor(sf::Color::White);
	playerText.setOutlineColor(sf::Color::Black);
	playerText.setOutlineThickness(1.0f);

	// Center the text in the panel
	float textX = playerPanel.getPosition().x + (playerPanel.getSize().x - playerText.getLocalBounds().width) / 2;
	float textY = playerPanel.getPosition().y + (playerPanel.getSize().y - playerText.getLocalBounds().height) / 2 - 5;
	playerText.setPosition(textX, textY);

	_game->window().draw(playerText);

	//sf::RectangleShape checkpointShape;
	for (size_t i = 0; i < _checkpoints.size(); i++) {
		// Color based on checkpoint status: green if reached, red if current, yellow if future
		if (_checkpoints[i].reached) {
			//SoundPlayer::getInstance().play("Checkpoint");
			checkpointShape.setFillColor(sf::Color(0, 0, 0, 0)); // Green for reached
			//checkpointShape.setOutlineColor(sf::Color::Green);
		}
		else if (i == _currentCheckpoint) {
			checkpointShape.setFillColor(sf::Color(0, 0, 0, 0)); // Red for current
			//checkpointShape.setOutlineColor(sf::Color::Red);
		}
		else {
			checkpointShape.setFillColor(sf::Color(0, 0, 0, 0)); // Yellow for future
			//checkpointShape.setOutlineColor(sf::Color::Yellow);
		}

		//checkpointShape.setOutlineThickness(2.0f);
		checkpointShape.setPosition(_checkpoints[i].area.left, _checkpoints[i].area.top);
		checkpointShape.setSize(sf::Vector2f(_checkpoints[i].area.width, _checkpoints[i].area.height));
		_game->window().draw(checkpointShape);


#ifdef DEBUG_CHECKPOINTS
		// Checkpoint marker rendering for debugging
		sf::RectangleShape checkpointMarker;
		checkpointMarker.setFillColor(sf::Color(0, 0, 0, 0)); // Transparent
		for (const auto& checkpoint : _checkpoints)
		{
			checkpointMarker.setPosition(checkpoint.area.left, checkpoint.area.top);
			checkpointMarker.setSize(sf::Vector2f(checkpoint.area.width, checkpoint.area.height));
			_game->window().draw(checkpointMarker);
		}
#endif
	}
}


void GameProject::initUI()
{
	// Load fonts
	if (!_uiFont.loadFromFile("assets/fonts/Audiowide-Regular.ttf")) {
		// Fallback to default font if custom font fails to load
		_uiFont = Assets::getInstance().getFont("main");
	}

	// Set up UI components

	// Timer display
	_timerText.setFont(_uiFont);
	_timerText.setCharacterSize(36);
	_timerText.setFillColor(sf::Color::White);
	_timerText.setOutlineColor(sf::Color(50, 50, 50));
	_timerText.setOutlineThickness(2.0f);

	// Barks counter
	_barkText.setFont(_uiFont);
	_barkText.setCharacterSize(28);
	_barkText.setFillColor(sf::Color::White);
	_barkText.setOutlineColor(sf::Color(50, 50, 50));
	_barkText.setOutlineThickness(2.0f);

	// Lap time display
	_lapInfoText.setFont(_uiFont);
	_lapInfoText.setCharacterSize(24);
	_lapInfoText.setFillColor(sf::Color::White);
	_lapInfoText.setOutlineColor(sf::Color(50, 50, 50));
	_lapInfoText.setOutlineThickness(1.5f);

	// Countdown text
	_countdownText.setFont(_uiFont);
	_countdownText.setCharacterSize(72);
	_countdownText.setFillColor(sf::Color::Red);
	_countdownText.setOutlineColor(sf::Color::White);
	_countdownText.setOutlineThickness(3.0f);

	// Create background for UI elements
	_uiBackground.setSize(sf::Vector2f(300, 50));
	_uiBackground.setFillColor(sf::Color(0, 0, 0, 150)); // Semi-transparent black
	_uiBackground.setOutlineColor(sf::Color(255, 255, 255, 100));
	_uiBackground.setOutlineThickness(1.0f);

	// Load UI icons
	//if (!_barkIconTexture.loadFromFile("assets/textures/bark_icon.png")) {
	//	// Create a simple colored square as fallback icon
	//	sf::Image img;
	//	img.create(32, 32, sf::Color::White);
	//	_barkIconTexture.loadFromImage(img);
	//}
	_barkIcon.setTexture(_barkIconTexture);
	_barkIcon.setScale(0.5f, 0.5f);
}



