#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>

enum Player_direction { stop, go_left, go_right }; // This for player's direction 

class projectile
{
public:
	//  Data members
	float velocity = 20;  // Bullet speed
	Player_direction direction;  //  1. Left,   2. Right
	sf::RectangleShape rect;  //  Bullets rectangle
	bool hit; //  Check if enemy was hit

	//  Methods
	projectile();
	void update();
};

