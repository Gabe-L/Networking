#include "pch.h"

#include "Input.h"
#include "Game.h"

void main()
{
	// Create the main window
	sf::RenderWindow window(sf::VideoMode(800, 600), "CS:2D");
	Input input;
	Game game(&window);

	sf::Clock clock;

	float deltaTime = 0.0f;
	float previousDeltaTime = 0.0f;

	while (window.isOpen())
	{
		// Get delta time
		//deltaTime = clock.restart().asSeconds();

		deltaTime = clock.getElapsedTime().asSeconds();
		deltaTime -= previousDeltaTime;
		previousDeltaTime = clock.getElapsedTime().asSeconds();

		// Handle events
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::Resized:
				window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
				break;
			case sf::Event::KeyPressed:
				input.setKeyDown(event.key.code);
				break;
			case sf::Event::KeyReleased:
				input.setKeyUp(event.key.code);
				break;
			case sf::Event::MouseMoved:
				input.setMousePosition(event.mouseMove.x, event.mouseMove.y);
				break;
			case sf::Event::MouseButtonPressed:
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					input.setLmbDown();
				}
				else
				{
					input.setRmbDown();
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					input.setLmbUp();
				}
				if (!sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					input.setRmbUp();
				}
				break;
			default:
				// don't handle other events
				break;
			}
		}

		// Update game
		game.handleInput(&input, deltaTime);
		game.update(deltaTime);
		game.render();

	}

}