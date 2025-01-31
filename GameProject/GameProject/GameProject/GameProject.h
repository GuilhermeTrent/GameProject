#pragma once

#include "Scene.h"
#include <queue>

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
    sPtrEntt                        _player{ nullptr };
    sf::View                        _worldView;
    sf::FloatRect                   _worldBounds;
    int                             _barkCounter{ 2 };
    sf::Text                        _barkText;


    LevelConfig                     _config;

    bool                            _drawTextures{ true };
    bool                            _drawAABB{ false };

    //systems
    void                    sAnimation(sf::Time dt);
    void                    sMovement(sf::Time dt);
    void                    sCollisions();
    void                    sUpdate(sf::Time dt);
    void	                onEnd() override;
    void                    sSpawnEnemies();
    void                    onBark();

    // helper functions
    void                    startAnimation(sPtrEntt e, std::string animation);
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

//private:
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

   /* sf::Text _barkText;
    sf::Font _barkFont;
    int _barkCounter = 2;

    bool canBark() const { return _barkCounter > 0; }*/
};

