// CS2D_Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "Simulation.h"

int main()
{
	sf::Clock clock;

	Simulation simulation;


	float deltaTime = 0.0f;
	float previousDeltaTime = 0.0f;

	while (simulation.update(deltaTime)) {
		deltaTime = clock.getElapsedTime().asSeconds();
		deltaTime -= previousDeltaTime;
		previousDeltaTime = clock.getElapsedTime().asSeconds();
	}
	
}