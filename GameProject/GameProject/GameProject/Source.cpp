////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Assignment:
//  Instructor:     David Burchill / Andrew Blakney
//  Year / Term:    Winter 2025
//  File name:      Source.cpp
//
//  Student name:   Guilherme Bouvie Trentini
//  Student email:  GBOUVIETRENTINI01@MYNBCC.CA
//
//     I certify that this work is my work only, any work copied from Stack Overflow, textbooks,
//     or elsewhere is properly cited.
//
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  BUG
//  list any and all bugs in your code
//  1. I didnt get abc to work because of xyz, I tried ijk...
//



#include <iostream>
#include "SplashScreen.h"
#include "GameEngine.h"
#include "Scene.h"




int main()
{

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Pug Race - Splash Screen");

   

    GameEngine game("../config.txt");
   /* std::shared_ptr<Scene> splashScene = std::make_shared<SplashScreen>(&game);
    game.changeScene("Splash", splashScene, true);*/
    game.run();
    return 0;
}