#include "projectile.h"

projectile::projectile()
{
	rect.setSize(sf::Vector2f(4, 2));
	rect.setFillColor(sf::Color::Black);
	rect.setPosition(0, 0);
	hit = false;
	direction = stop;
}

void projectile::update()
{
	if (direction == 1)
		rect.move(-velocity, 0);  // Right

	if (direction == 2)
		rect.move(velocity, 0);  //  Left
}
