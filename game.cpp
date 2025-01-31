#include "game.hpp"
#include <fstream>
#include <iostream>

Game::Game(){
	music = LoadMusicStream("Sounds/music.ogg");
	explosionSound = LoadSound("Sounds/explosion.ogg");
	PlayMusicStream(music);
	InitGame();
}

Game::~Game() {
	Alien::UnloadImages();
	UnloadMusicStream(music);
	UnloadSound(explosionSound);
}

void Game::Update() {
	if (run) {
		CheckForNewWave();
		double currentTime = GetTime();
		if (currentTime - lastTimeSpawn > mysteryShipSpawnInterval) {
			mysteryship.Spawn();
			lastTimeSpawn = GetTime();
			mysteryShipSpawnInterval = GetRandomValue(10, 20);
		}

		for (auto& laser : spaceship.lasers) {
			laser.Update();
		}
		MoveAliens();
		AlienShootLaser();

		for (auto& laser : alienLasers) {
			laser.Update();
		}
		DeleteInactiveLasers();
		mysteryship.Update();
		CheckForCollisions();
	}
	else {
		if (IsKeyDown(KEY_ENTER)) {
			Reset();
			InitGame();
		}
	}
}

void Game::Draw() {
	spaceship.Draw();
	for (auto& laser : spaceship.lasers) {
		laser.Draw();
	}

	for (auto& obstacle : obstacles) {
		obstacle.Draw();
	}

	for (auto& alien : aliens) {
		alien.Draw();
	}

	for (auto& laser : alienLasers) {
		laser.Draw();
	}
	mysteryship.Draw();
}

void Game::HandleInput() {
	if (run) {
		if (IsKeyDown(KEY_A)) {
			spaceship.MoveLeft();
		}
		else if (IsKeyDown(KEY_D)) {
			spaceship.MoveRight();
		}
		else if (IsKeyDown(KEY_SPACE)) {
			spaceship.FireLaser();
		}
	}
}

void Game::CheckForNewWave(){
	if (aliens.size() == 0) {
		lives = 3;
		alienLasers.clear();
		aliens.clear();
		aliens = CreateAliens();
		alienLaserShootInterval -= 0.02;
		levelNumber++;
		if (levelNumber < 5) {
			obstacles.clear();
			obstacles = CreateObstacles();
		}
	}
}

void Game::DeleteInactiveLasers(){
	for (auto it = spaceship.lasers.begin(); it != spaceship.lasers.end(); ) {
		if (!it->active) it = spaceship.lasers.erase(it);
		else ++it;
	}

	for (auto it = alienLasers.begin(); it != alienLasers.end(); ) {
		if (!it->active) it = alienLasers.erase(it);
		else ++it;
	}
}

std::vector<Obstacle> Game::CreateObstacles(){
	int obstacleWidth = Obstacle::grid[0].size() * 3;
	float gap = (GetScreenWidth() - (4 * obstacleWidth)) / 5;

	for (int i = 0; i < 4; i++) {
		float offsetX = (i + 1) * gap + i * obstacleWidth;
		obstacles.push_back(Obstacle({ offsetX, float(GetScreenHeight() - 200) }));
	}
	return obstacles;
}

std::vector<Alien> Game::CreateAliens(){
	std::vector<Alien> aliens;
	for (int row = 0; row < 5; row++) {
		for (int column = 0; column < 11; column++) {
			int alienType;
			if (row == 0) {
				alienType = 3;
			}
			else if (row == 1 or row == 2) {
				alienType = 2;
			}
			else {
				alienType = 1;
			}

			float x = 75 + column * 55;
			float y = 110 + row * 55;
			aliens.push_back(Alien(alienType, {x, y}));
		}
	}
	return aliens;
}

void Game::MoveAliens() {
	for (auto& alien : aliens) {
		if (alien.position.x + alien.alienImages[alien.type - 1].width > GetScreenWidth() - 25) {
			aliensDirection = -1;
			MoveDownAliens(4);
		}
		if (alien.position.x < 25) {
			aliensDirection = 1;
			MoveDownAliens(4);
		}
		alien.Update(aliensDirection);
	}
}

void Game::MoveDownAliens(int distance){
	for (auto& alien: aliens) {
		alien.position.y += distance;
	}
}

void Game::AlienShootLaser() {
	double currentTime = GetTime();
	if (currentTime - timeLastAlienFired >= alienLaserShootInterval && !aliens.empty()) {
		int randomIndex = GetRandomValue(0, aliens.size() - 1);
		Alien& alien = aliens[randomIndex];
		alienLasers.push_back(Laser({ alien.position.x + alien.alienImages[alien.type - 1].width / 2,
									  alien.position.y + alien.alienImages[alien.type - 1].height }, 6));
		timeLastAlienFired = GetTime();
	}
}

void Game::CheckForCollisions(){
	//Player laser collision
	for (auto& laser : spaceship.lasers) {
		auto it = aliens.begin();
		while (it != aliens.end()) {
			if (CheckCollisionRecs(it->GetRect(), laser.GetRect())) {
				PlaySound(explosionSound);
				if (it->type == 1) {
					score += 100;
				}
				else if (it->type == 2) {
					score += 200;
				}
				else if (it->type == 3) {
					score += 300;
				}
				CheckForHighScore();
				it = aliens.erase(it);
				laser.active = false;
			}
			else ++it;
		}
		for (auto& obstacle : obstacles) {
			auto it = obstacle.blocks.begin();
			while (it != obstacle.blocks.end()) {
				if (CheckCollisionRecs(it->GetRect(), laser.GetRect())) {
					it = obstacle.blocks.erase(it);
					laser.active = false;
				}
				else ++it;
			}
		}
		if (CheckCollisionRecs(mysteryship.GetRect(), laser.GetRect())) {
			PlaySound(explosionSound);
			mysteryship.alive = false;
			laser.active = false;
			score += 500;
			CheckForHighScore();
		}
	}
	//Aliens lasers collision
	for (auto& laser : alienLasers) {
		if (CheckCollisionRecs(laser.GetRect(), spaceship.getRect())) {
			laser.active = false;
			lives--;
			if (lives == 0) {
				GameOver();
			}
		}

		for (auto& obstacle : obstacles) {
			auto it = obstacle.blocks.begin();
			while (it != obstacle.blocks.end()) {
				if (CheckCollisionRecs(it->GetRect(), laser.GetRect())) {
					it = obstacle.blocks.erase(it);
					laser.active = false;
				}
				else ++it;
			}
		}
	}
	//Allien collision with obstacles
	for (auto& alien : aliens) {
		for (auto& obstacle : obstacles) {
			auto it = obstacle.blocks.begin();
			while (it != obstacle.blocks.end()) {
				if (CheckCollisionRecs(it->GetRect(), alien.GetRect())) {
					it = obstacle.blocks.erase(it);
				}
				else ++it;
			}
		}

		if (CheckCollisionRecs(alien.GetRect(), spaceship.getRect())) {
			GameOver();
		}
	}
}

void Game::GameOver(){
	run = false;
}

void Game::Reset(){
	spaceship.Reset();
	aliens.clear();
	alienLasers.clear();
	obstacles.clear();
}

void Game::InitGame(){
	obstacles = CreateObstacles();
	aliens = CreateAliens();
	aliensDirection = 1;
	timeLastAlienFired = 0.0;
	lastTimeSpawn = 0.0;
	lives = 3;
	run = true;
	mysteryShipSpawnInterval = GetRandomValue(10, 20);
	score = 0;
	highScore = loadHighScoreFromFile();
	levelNumber = 1;
}

void Game::CheckForHighScore(){
	if (score > highScore) {
		highScore = score;
		saveHighScoreToFile(highScore);
	}
}

void Game::saveHighScoreToFile(int highScore){
	std::ofstream highScoreFile("highscore.txt");
	if (highScoreFile.is_open()) {
		highScoreFile << highScore;
		highScoreFile.close();
	}
}

int Game::loadHighScoreFromFile(){
	int loadedHighScore = 0;
	std::ifstream highScoreFile("highscore.txt");
	if (highScoreFile.is_open()) {
		highScoreFile >> loadedHighScore;
		highScoreFile.close();
	}
	return loadedHighScore;
}
