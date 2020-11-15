#include <SFML/Graphics.hpp>  //  This for all the graphics
#include <SFML/Audio.hpp>  //  This for sounds
#include <vector>  //  This handles the projectile bullets
#include "TileMap.h"	//  Map is in here
#include "projectile.h"
#include <algorithm> // For erase remove idiom
#include <sstream> // For converting score int to string text

#define size 60  //  Size of the army of enemies

int coin_count = 0;	// Score
int enemy_kills = 0; // Killings

using namespace sf;	 //  SFML namespace   
using namespace std;

float offsetX = 0, offsetY = 0;  //  Screen offset variables
bool isGun = false;


class Base  //  Virtual Base class for player and enemies
{
public:  //  Common data members for all things that will move
	float dx, dy, currentFrame;  // Displacement(dx & dy) and frame variables 
	FloatRect rect;	 // Rectangle that will hold objects and move in the map
	Sprite sprite; // Object sprite
	int power;
	bool life, onGround; // OnGround checks if landed
	Player_direction direction;	// enum to get direction
	RectangleShape power_indicator; //  Power Indicator for Enemies and Mario

public:
	virtual void update(float time) = 0; //  This is the virtual update function
	virtual void Collision(int num) = 0; //  Virtual Collision function 
	virtual void operator --(int) = 0;	 // To reduce power
};

class PLAYER : public Base
{
private:

public:
	Player_direction direction;  //  1. Left,   2. Right (Player's direction as an enum)

	// Break wall sound buffer
	sf::SoundBuffer buffer1;
	sf::Sound wallbreak;

	//  Coin collect sound here
	SoundBuffer coinBuffer;
	Sound coin_collected;

	//  Gun reload sound effect
	sf::SoundBuffer buffer2;
	sf::Sound Reload;
	bool reloadPlay = true;

	PLAYER(Texture &image) //  Player's CTor
	{
		sprite.setTexture(image);
		rect = FloatRect(100, 180, 16, 16);	 //  Mario's sprite size

		power_indicator.setSize(Vector2f(20, 4));  //  Power indicator set
		power_indicator.setFillColor(Color::Green);  //  Power indicator's Color set
		power_indicator.setPosition(0, 0);  //  Set postion for indicator

		// Break wall sound effect
		buffer1.loadFromFile("Concretebreak.ogg");
		wallbreak.setBuffer(buffer1);
		wallbreak.setVolume(100);

		// Reload sound effect
		buffer2.loadFromFile("Reload.wav");
		Reload.setBuffer(buffer2);
		Reload.setVolume(100);

		// Coin collect sound effect
		coinBuffer.loadFromFile("coinCollect.wav");
		coin_collected.setBuffer(coinBuffer);
		coin_collected.setVolume(100);

		dx = dy = 0.1f;
		currentFrame = 0;  //  First frame
		direction = stop;  // Player has no direction at the time of instantiation
		power = 20;
		onGround = false;
	}

	//  Player's update function
	void update(float time)
	{
		rect.left += dx * time;
		Collision(0); // For dx

		if (!onGround) dy = dy + 0.0005f * time + 0.003;  // Jump hieght controlled here (Add a very small float to lower jump hieght)
		rect.top += dy * time;
		onGround = false;
		Collision(1); // For dy

		currentFrame += time * 0.005f;
		if (currentFrame > 3) currentFrame -= 3;

		// Mirror the sprite if direction is reversed and update the sprite
		if (dx > 0) sprite.setTextureRect(IntRect(112 + 31 * int(currentFrame), 144, 16, 16));
		if (dx < 0) sprite.setTextureRect(IntRect(112 + 31 * int(currentFrame) + 16, 144, -16, 16));

		sprite.setPosition(rect.left - offsetX, rect.top - offsetY); // Set Mario's position

		//Update power
		if (power >= 20 && life)
			power = 20;

		if (!life)
			power = 0;	// Dead

		if (power <= 8) power_indicator.setFillColor(Color::Red);
		else
			power_indicator.setFillColor(Color::Green);
		power_indicator.setSize(Vector2f(power, 4.f)); // Update power indicator
		power_indicator.setPosition(rect.left - offsetX, rect.top - offsetY - 10);  //  Set postion for indicator

		dx = 0;
	}

	void Collision(int num)
	{
		for (int i = rect.top / 16; i < (rect.top + rect.height) / 16; i++)
			for (int j = rect.left / 16; j < (rect.left + rect.width) / 16; j++)
			{
				if ((TileMap1[i][j] == 'P') || (TileMap1[i][j] == 'k') || (TileMap1[i][j] == '0') || (TileMap1[i][j] == 'r') || (TileMap1[i][j] == 't') || (TileMap1[i][j] == 'c') || (TileMap1[i][j] == 'b') || (TileMap2[i][j] == 'u'))
				{
					if (dy > 0 && num == 1)
					{
						rect.top = i * 16.f - rect.height;
						dy = 0;
						onGround = true;
					}

					if (dy < 0 && num == 1)
					{
						rect.top = i * 16.f + 16.f;
						dy = 0;
						if (TileMap1[i][j] == 'k' || TileMap1[i][j] == 'b')    //     Wall breaking here
						{
							power--; // Reduce and balance power
							if (power <= 0) { power = 0; life = false; }

							TileMap1[i][j] = ' ';
							wallbreak.play();
						}
						if (TileMap1[i][j] == 'c')   //    Pickables here
						{
							TileMap1[i][j] = ' ';  // Clear it any way

							if (rand() % 2 == 1 && reloadPlay)
							{
								Reload.play();
								reloadPlay = false;
								isGun = true;
							}
							else if (life) // Increase and balance power if it's not dead
								power += 10;
						}
					}
					if (dx > 0 && num == 0)
					{
						rect.left = j * 16.f - rect.width;
					}

					if (dx < 0 && num == 0)
					{
						rect.left = j * 16.f + 16.f;
					}
				}
				if (TileMap2[i][j] == 'u') // Check for coin collect
				{
					TileMap2[i][j] = ' ';
					coin_count++;
					coin_collected.play();
				}
			}
	}

	// Over ride reducePower virtual operator
	void operator --(int)
	{
		power--;
	}

};

class ENEMY : public Base
{
public:
	void set(Texture &image, int x, int y)
	{
		power_indicator.setSize(Vector2f(20, 4));  //  Power indicator set
		power_indicator.setFillColor(Color::Green);  //  Power indicator's Color set
		power_indicator.setPosition(0, 0);  //  Set postion for indicator

		sprite.setTexture(image);  //  enemy's sprite
		rect = FloatRect(float(x), float(y), 16.f, 16.f);	 //  Texture rect size and position

		dx = 0.05f;
		dy = 0.1f;
		currentFrame = 0;
		life = true;
		power = 10;
		onGround = false;
	}

	void update(float time)
	{
		rect.left += dx * time;
		Collision(0);

		if (dy > 0 || !onGround)
		{
			dy = dy + 0.0005f * time;  // Jump hieght controlled here (Add a very small float to lower jump hieght)
			rect.top += dy * time;
			onGround = false;
			Collision(1);
		}

		currentFrame += time * 0.005f;
		if (currentFrame > 2.f) currentFrame -= 2.f;

		sprite.setTextureRect(IntRect(18 * int(currentFrame), 0, 16, 16));
		if (!life) sprite.setTextureRect(IntRect(58, 0, 16, 16));

		sprite.setPosition(rect.left - offsetX, rect.top - offsetY); // Set enemy postion

		if (!life) power = 0; // Zero power if player is dead
		if (power <= 4) power_indicator.setFillColor(Color::Red);
		power_indicator.setSize(Vector2f(power * 2.f, 4.f)); // Since power initially is 10 and length of indicator is 20
		power_indicator.setPosition(rect.left - offsetX, rect.top - offsetY - 10);  //  Set postion for indicator

	}

	void Collision(int num)
	{
		for (int i = rect.top / 16; i < (rect.top + rect.height) / 16; i++)
			for (int j = rect.left / 16; j < (rect.left + rect.width) / 16; j++)
			{
				if ((TileMap1[i][j] == 'P') || (TileMap1[i][j] == 'k') || (TileMap1[i][j] == '0') || (TileMap1[i][j] == 'r') || (TileMap1[i][j] == 't') || (TileMap1[i][j] == 'c') || (TileMap1[i][j] == 'b'))
				{
					if (dy > 0 && num == 1)
					{
						rect.top = i * 16.f - rect.height;
						dy = 0;
						onGround = true;
					}

					if (dy < 0 && num == 1)
					{
						rect.top = i * 16.f + 16.f;
						dy = 0;
					}
					// Original one

					if (dx > 0)
					{
						rect.left = j * 16.f - rect.width;	dx *= -1;
					}

					else if (dx < 0)
					{
						rect.left = j * 16.f + 16.f;  dx *= -1;
					}
				}
			}
	}
	// Over ride reducePower virtual operator
	void operator --(int)
	{
		power--;
	}
};

class coin : public Base
{
private:
	int width = 50, hieght = width;

public:
	// Coin Ctor
	coin(const Texture& image, int x, int y)
	{
		sprite.setTexture(image);
		currentFrame = 0;
		//rect = FloatRect(x, y, width, hieght);  // Postion of coin sprites
		sprite.setTextureRect(IntRect(currentFrame * width, 0, width, hieght));  // size of coin sprites
		sprite.setScale(0.3, 0.3);
	}

	// Over ride update
	void update(float time)
	{
		// Update coin sprite
		sprite.setTextureRect(IntRect(currentFrame * width, 0, width, hieght));

		if (rand() % 3 == 1) // To slow down the rate of sprite update
			currentFrame++;	 // increment frames
		if (currentFrame >= 10) currentFrame = 0;  // Reset currentFrame

		sprite.setPosition(rect.left - offsetX, rect.top - offsetY); // Set coin position
	}

	// Over ride the Collision function without any body (Since it is not required)
	void Collision(int num) {}

	// Over ride reducePower virtual operato without any body (Since it is not required)
	void operator --(int) {}
};

void printMap(Sprite& tile, Sprite& coins, RenderWindow& window)
{
	//  Coins in here 
	for (int i = 0; i < H; i++)
		for (int j = 0; j < W; j++)
		{
			if (TileMap1[i][j] == 'u')  coins.setTextureRect(IntRect(0, 0, 50, 50));  //   Coins
			if (TileMap2[i][j] == ' ' || TileMap2[i][j] == '0' || TileMap2[i][j] == 'r') continue;

			coins.setPosition(j * 16 - offsetX, i * 16 - offsetY);
			window.draw(coins);
		}

	//  ForeScreen Tiles
	for (int i = 0; i < H; i++)
		for (int j = 0; j < W; j++)
		{
			if (TileMap1[i][j] == 'P')  tile.setTextureRect(IntRect(143 - 16 * 3, 112, 17, 17));  //   Ground Tiles
			if (TileMap1[i][j] == 'k')  tile.setTextureRect(IntRect(143, 112, 16, 16));  //   Air Bricks
			if (TileMap1[i][j] == 'w')  tile.setTextureRect(IntRect(99, 224, 140 - 99, 255 - 224));  //  Clouds
			if (TileMap1[i][j] == 'b')  tile.setTextureRect(IntRect(143, 112 + 16, 16, 16));  //   Air Bricks
			if (TileMap1[i][j] == 'c')  tile.setTextureRect(IntRect(143 - 16, 112, 16, 16));  //    Question marks
			if (TileMap1[i][j] == 't')  tile.setTextureRect(IntRect(0, 47, 32, 95 - 47));  //   Tunnels 
			if (TileMap1[i][j] == 'r')  tile.setTextureRect(IntRect(143 - 32, 112, 16, 16));  //   Stair Bricks
			if (TileMap1[i][j] == ' ' || TileMap1[i][j] == '0') continue;

			tile.setPosition(j * 16 - offsetX, i * 16 - offsetY);
			window.draw(tile);

		}
}

int main()
{
	RenderWindow welcom(VideoMode(400, 250), "Welcome");
	// Welcome sprite
	Texture welc;
	welc.loadFromFile("Materials/MarioGameStarter.png");
	Sprite welcome(welc);
	welcome.setScale(0.5, 0.38);
	bool check = false; // To keep check on it

	/*while (welcom.isOpen())
	{
		Event event;
		while (welcom.pollEvent(event))
		{
			if (event.type == event.Closed)
				welcom.close();
		}

		if (Keyboard::isKeyPressed(Keyboard::Return) && !check) { check = true; continue; } // to go to instructions
		if (Keyboard::isKeyPressed(Keyboard::P)) welcom.close(); // To Begin Playing

		if (check) // Instructions
		{
			welc.loadFromFile("Materials/Instructions.png");
			welcome.setTexture(welc);
			welcome.setScale(0.7, 0.8);
		}

		welcom.clear(Color::White);
		welcom.draw(welcome);
		welcom.display();
	}*/

	// Game begins here

	//  Initiating window and setting its size and title
	RenderWindow window(VideoMode(400, 250), "Materials/Mario is Back");

	//  Fore ground Tiles
	Texture tileSet;
	tileSet.loadFromFile("Materials/Mario_Tileset.png");
	Sprite tile(tileSet);

	//  Mario instantiated by implementing polymorphism
	Base *pointer_to_Mario = new PLAYER(tileSet);

	//  Create an army of enemies again by polymorphism
	ENEMY enemies[size];
	for (size_t i = 0; i < size; i++)
	{
		if (i == 0) enemies[i].set(tileSet, 220 * i + 200, 13 * 6);
		else if (i % 2 == 0)
			enemies[i].set(tileSet, 220 * i + 100, 13 * 6);  // In air
		else
			enemies[i].set(tileSet, 220 * i + 100, 13 * 16);  // On ground
	}

	/*//  Change color of all enemies
	for (size_t i = 0; i < size; i++)
	{
		enemies[i].sprite.setColor(Color::Blue);
	}*/

	// Coins Texture
	Texture coins_texture;
	coins_texture.loadFromFile("Materials/coins.png");

	// Coins from base pointers, Ploymorphism, and also place all the coins 
	Base *coins/*[num_of_coins]*/;
	coins = new coin(coins_texture, 200 * 1 + 200, 13 * 16);

	//  Gun Texture and sprite
	Texture ForGun;
	ForGun.loadFromFile("gun.png");
	Sprite gun(ForGun);
	gun.setScale(0.4, 0.2);
	gun.setPosition(pointer_to_Mario->rect.left - offsetX, pointer_to_Mario->rect.top - offsetY + 5);

	//  Mario's jump sound
	SoundBuffer buffer;
	buffer.loadFromFile("Jump.ogg");
	Sound sound(buffer);
	sound.setVolume(40);

	//  Crushing enemy sound
	sf::SoundBuffer buffer1;
	buffer1.loadFromFile("SMACK.ogg");
	sf::Sound smack(buffer1);
	smack.setVolume(100);

	//  Bullet shot sound
	SoundBuffer buffer2;
	buffer2.loadFromFile("hithard.wav");
	Sound bulletsound(buffer2);
	bulletsound.setVolume(40);

	//  Background music here
	Music music;
	music.openFromFile("Fantasy.wav");
	music.play();
	music.setVolume(100);
	music.setLoop(true);//  Set to loop infinitely

	// Text to display score
	Font TimesNewRoman;
	TimesNewRoman.loadFromFile("timesbd.ttf");  // Bold
	Text Score;
	Score.setFont(TimesNewRoman);
	Score.setCharacterSize(20);
	Score.setPosition(10, 20);
	
	

	//  Bullet controller
	int counter;

	// Projectile vector
	vector<projectile>::const_iterator iter;
	vector<projectile> projectilearray;
	projectile projectile1;


	// Predicate in order to get a single arguement function to implement theerase remove idiom
	auto isHit = [](const projectile& proj) { return proj.hit; };//  Auto function

	//  For controlling window resize
	Clock clock;
	while (window.isOpen() && pointer_to_Mario->life)  //  Game in here
	{
		//  Control window resizing and making it's dependence on time elapsed
		float time = clock.getElapsedTime().asMicroseconds();
		clock.restart();
		time = time / 500;

		//  Event handling (Manipulating Key Strokes here)
		Event event;
		while (window.pollEvent(event))
		{
			//   Check for cross and ESC key press
			if (event.type == Event::Closed || event.key.code == Keyboard::Escape)
				window.close();

			//   Generate and push_back bullets into projectile array and produce bullet's sound
			if (event.key.code == 'x' && isGun == true)
			{
				projectile1.rect.setPosition(pointer_to_Mario->sprite.getPosition().x, pointer_to_Mario->sprite.getPosition().y + 7);
				projectile1.direction = pointer_to_Mario->direction;
				projectilearray.push_back(projectile1);	//  bullet going into the vector
				bulletsound.play();
			}
		}

		//   Control Mario's Motion	and his gun
		if (Keyboard::isKeyPressed(Keyboard::Left)) { pointer_to_Mario->dx = -0.1; pointer_to_Mario->direction = go_left; }
		if (Keyboard::isKeyPressed(Keyboard::Right)) { pointer_to_Mario->dx = 0.1; pointer_to_Mario->direction = go_right; }
		if (Keyboard::isKeyPressed(Keyboard::Up))
			if (pointer_to_Mario->onGround)
			{
				pointer_to_Mario->dy = -0.37f;
				pointer_to_Mario->onGround = false;
				sound.play();
			}

		//   Load Texture for Mario's Gun here
		Texture ForGun;
		ForGun.loadFromFile("gun.png");
		Sprite gun(ForGun);

		//   Two ifs that save Mario's direction and synchronize Gun's orientation with it.
		if (pointer_to_Mario->direction == go_right)
		{
			gun.setScale(0.4f, 0.2f);
			gun.setPosition(pointer_to_Mario->rect.left - offsetX, pointer_to_Mario->rect.top - offsetY + 5);
		}
		if (pointer_to_Mario->direction == go_left)
		{
			gun.setScale(-0.4f, 0.2f);
			gun.setPosition(pointer_to_Mario->rect.left - offsetX + 18, pointer_to_Mario->rect.top - offsetY + 5);
		}

		// Check if a bullet goes out of screen bounds and delete it
		counter = 0;
		for (iter = projectilearray.begin(); iter != projectilearray.end() && isGun == true; iter++)
		{
			if (!projectilearray[counter].hit && (projectilearray[counter].rect.getPosition().x > 400 || projectilearray[counter].rect.getPosition().x < -400))  //  Bullet has not hit anything
			{
				projectilearray[counter].hit = true; //  Bullet went out of screen vicinity.
			}
			counter++;
		}
		//  Erase remove idiom to delete bullets
		projectilearray.erase(remove_if(projectilearray.begin(), projectilearray.end(), isHit), projectilearray.end());

		// Check if the enemies are hit with Bullets
		counter = 0;
		for (iter = projectilearray.begin(); iter != projectilearray.end() && isGun == true; iter++)
		{
			for (size_t i = 0; i < size; i++)
			{
				if (projectilearray[counter].rect.getGlobalBounds().intersects(enemies[i].sprite.getGlobalBounds()) && enemies[i].life
					&& !projectilearray[counter].hit)
				{
					projectilearray[counter].hit = true; //  Enemy was hit once 
					enemies[i]--; //  Decrease enemy's power using this method

					// Ckeck if all power has been drained then kill the enemy
					if (enemies[i].power <= 0)
					{
						enemies[i].dx = 0;  //  Stop the enemy 
						enemies[i].life = false;  //  It's dead
						enemy_kills++;
					}

				}
			}
			counter++;
		}
		//  Erase remove idiom to delete bullets
		projectilearray.erase(remove_if(projectilearray.begin(), projectilearray.end(), isHit), projectilearray.end());

		//   Update Mario
		pointer_to_Mario->update(time);

		//   Update all enemies
		for (size_t i = 0; i < size; i++)
		{
			enemies[i].update(time);
		}

		// update all coins
		coins->update(time);

		//   Check Mario and Enemy Collisions here.
		for (size_t i = 0; i < size; i++)
		{
			if (pointer_to_Mario->rect.intersects(enemies[i].rect))
			{
				if (enemies[i].life) {
					if (pointer_to_Mario->dy > 0) //  Mario kills enemy by crushing it
					{
						enemies[i].dx = 0;  //  Stop the enemy
						pointer_to_Mario->dy = -0.2f;  //  Mario gets a jump
						enemies[i].power = 0;  //  Set power to zero 
						enemies[i].life = false;  // Set status to dead
						enemy_kills++;
						smack.play(); //  Play Crushing sound
					}
					else
					{
						pointer_to_Mario->sprite.setColor(Color::Red);  //   Mario dies here
						pointer_to_Mario->life = false;
					}
				}
			}
		}

		//  Screen scrolling here
		if (pointer_to_Mario->rect.left > 200) offsetX = pointer_to_Mario->rect.left - 200;
		//if (pointer_to_Mario->rect.top < 120) offsetY = pointer_to_Mario->rect.top - 120;

		window.clear(Color::Cyan);  // Back ground Color

		//   Print all the map here.
		printMap(tile, coins->sprite, window);

		//   Draw Mario here
		window.draw(pointer_to_Mario->sprite);

		//   Draw Mario's Power indicator here
		//window.draw(pointer_to_Mario->power_indicator);

		//  Draw all enemies power indicators here
		/*for (size_t i = 0; i < size; i++)
		{
			window.draw(enemies[i].power_indicator);
		}*/

		//   Check if the gun has been acquired and draw it if true.
		//if (isGun)
			//window.draw(gun);

		//Update and draw all projectile bullets
		counter = 0;
		for (iter = projectilearray.begin(); iter != projectilearray.end() && isGun == true; iter++)
		{
			projectilearray[counter].update();  //   Update bullets
			window.draw(projectilearray[counter].rect);  //  Draw bullets

			counter++;
		}

		//  Draw all enemies here
		for (size_t i = 0; i < size; i++)
		{
			window.draw(enemies[i].sprite);
		}

		// Draw all coins
		window.draw(coins->sprite);

		// this to convert int score to text
		stringstream ss;

		// Get score to draw it
		ss << ((enemy_kills + coin_count) * 100);
		// Set int to string into Text
		Score.setString("Score: " + ss.str());

		// Draw score
		window.draw(Score);

		//  Display everything on the screen
		window.display();

		if (!pointer_to_Mario->life)
			window.close();			  // Close window if mario's dead

	}

	//  delete the player and the coins
	delete pointer_to_Mario;
	delete coins;

	// Game ends here

	// GameOver
	RenderWindow GameOver(VideoMode(400, 250), "Game Ends Here");
	// Game Over sprite
	Texture over;
	if ((enemy_kills + coin_count) * 100 <= 25000)
		over.loadFromFile("Materials/GameOver.png");	// If lose
	else
		over.loadFromFile("Materials/YouWin.png");  // If win

	Sprite gameOver(over);

	while (GameOver.isOpen())
	{
		Event event;
		while (GameOver.pollEvent(event))
		{
			if (event.type == event.Closed || event.key.code == Keyboard::Return || event.key.code == Keyboard::Escape)
				GameOver.close();
		}

		GameOver.clear();
		GameOver.draw(gameOver);
		GameOver.display();
	}

	return 0;
}