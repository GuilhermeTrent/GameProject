#include <iostream>
#include "SplashScreen.h"
#include "GameEngine.h"
#include "Scene.h"

int main()
{
    GameEngine game("../config.txt");

    // Create and set the SplashScreen as the initial scene
    //std::shared_ptr<Scene> splashScene = std::make_shared<SplashScreen>(&game);
    //game.changeScene("SPLASH", splashScene);

    game.run();
    return 0;
}