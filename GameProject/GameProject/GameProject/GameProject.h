#pragma once

#include "Scene.h"
#include <queue>


struct Checkpoint {
    sf::FloatRect area;
    bool reached = false;
};

struct BlockingSquare {
    int x, y;
    int width = 10;  // Default size of the blocking square
    int height = 10;
    sf::FloatRect bounds;
    
};

//struct BlockingSquare {
//    int x, y;
//    int width = 10;  // Default size of the blocking square
//    int height = 10;
//    CBoundingBox boundingBox;
//
//    BlockingSquare(float x, float y, float w, float h)
//        : x(x), y(y), width(w), height(h), boundingBox(w, h) {}
//};

//extern std::vector<BlockingSquare> blockingSquares;
//extern Player player;

void generateBlockingSquares();



extern std::vector<BlockingSquare> obstacles;




struct SpawnPoint {
    std::string type;
    float       y;

    auto operator<=>(const SpawnPoint& other) const {
        return y <=> other.y;
    }
};

struct LevelConfig {
    float       playerSpeed{ 200.f };
};


class GameProject : public Scene
{
    GameEngine*                      m_game;
    sPtrEntt                        _player{ nullptr };
    sf::View                        _worldView;
    sf::FloatRect                   _worldBounds;
    int                             _barkCounter{ 2 };
    sf::Text                        _barkText;


    LevelConfig                     _config;

    bool                            _drawTextures{ true };
    bool                            _drawAABB{ false };

    sf::Clock _lapTimer;  // Timer to track lap time
    float _lastLapTime = 0.0f;  // Stores last completed lap time
    int _lapCount = 0;

    std::vector<Checkpoint> _checkpoints;
    sf::FloatRect _finishLine;
    int _currentCheckpoint = 0;
    bool _allCheckpointsReached = false;
   // bool _lapCompleted = false;
    //int _lapCount = 0;

    void checkLapProgress();

    void setupCheckpoints(const std::string& levelPath);

    void resetLapProgress();
    void generateBlockingSquares();
    void initializeObstacles();
    void initializeCheckpoints();
    void updateUI();


    //systems
    void                    sAnimation(sf::Time dt);
    void                    sMovement(sf::Time dt);
    void                    sCollisions();
    void                    sUpdate(sf::Time dt);
    void	                onEnd() override;
    void                    sSpawnEnemies();
    void                    onBark();

    // helper functions
//  //  void spawnBarrels();
void checkBarkCollision();
void startAnimation(sPtrEntt e, std::string animation);
   // void                    startAnimation(sPtrEntt e, std::string animation);
    void                    checkIfDead(sPtrEntt e);
    void                    checkPlayerCollision();
    void                    destroyOutsideWindow();
    void                    spawnEnemy(SpawnPoint sp);
    void	                registerActions();
    void                    spawnPlayer(sf::Vector2f pos);
    void                    playerMovement();
    void                    annimatePlayer();
    void                    adjustPlayerPosition();
    void                    init(const std::string& path);
    void                    loadLevel(const std::string& path);
    void                    updateBarkText();
    void                    spawnBarrel();
    void                    handleBarking();
    void                    spawnBone();

  //  std::vector<std::shared_ptr<Entity>> _obstacles;

    std::vector<std::shared_ptr<Entity>> _barrels;
    bool _barrelsSpawned = false;

    std::vector<std::shared_ptr<Entity>> _bones;
    bool _bonesSpawned = false;

    float m_countdownTime = 3.0f; // Countdown before race starts
    float m_raceTime = 0.0f;      // Race time after countdown
    bool m_timerActive = false;   // Track when the race is ongoing

    sf::Text _timerText;
    sf::Text m_countdownText;
    std::shared_ptr<Entity> _backgroundEntity;
    sf::Image _backgroundImage;
    sf::Image _backgroundImageBeach;
    sf::Image _backgroundImageSnow;

    bool _playerSpeedBoost = false;
    float _speedBoostTimer = 0.0f;
    float _deltaTime = 0.016f; 

private:
    bool _enableSnow = false;
    std::string _levelPath;
//    sf::Text _barkText;        
//    sf::Font _barkFont;           
//    int _barkCounter = 2;   
//
    bool canBark() const { return _barkCounter > 0; }
  

public:
    GameProject(GameEngine* gameEngine, const std::string& levelPath);

    

    void		            update(sf::Time dt) override;
    void		            sDoAction(const Command& command) override;
    void		            sRender() override;
    void spawnPlayerForLevel();

   /* sf::Text _barkText;
    sf::Font _barkFont;
    int _barkCounter = 2;

    bool canBark() const { return _barkCounter > 0; }*/
};

