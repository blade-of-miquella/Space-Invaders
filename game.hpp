#pragma once
#include "spaceship.hpp" 
#include "obstacle.hpp"
#include "alien.hpp"
#include "mysteryship.hpp"

class Game {
public:
	Game();
	~Game();
	void Draw();
	void Update();
	void HandleInput();
	void CheckForNewWave();
	int lives;
	bool run;
	int score;
	int highScore;
	Music music;
	int levelNumber;
private:
	void DeleteInactiveLasers();
	std::vector<Obstacle> CreateObstacles();
	std::vector<Alien> CreateAliens();
	void MoveAliens();	
	void MoveDownAliens(int distance);
	void AlienShootLaser();
	void CheckForCollisions();
	void GameOver();
	void Reset();
	void InitGame();
	void CheckForHighScore();
	void saveHighScoreToFile(int highScore);
	int loadHighScoreFromFile();
	Spaceship spaceship;
	std::vector<Obstacle> obstacles;
	std::vector<Alien> aliens;
	int aliensDirection;
	std::vector<Laser> alienLasers;
	float alienLaserShootInterval = 0.6;
	float timeLastAlienFired;
	MysteryShip mysteryship;
	float mysteryShipSpawnInterval;
	float lastTimeSpawn;
	Sound explosionSound;
};