#include "game.h"

void Game::setUp()
{
	// Windows initialization
	window.create(sf::VideoMode(850, 600), "Game");

	if (!font.loadFromFile("resources/fonts/courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	bg.loadFromFile("resources/bg.png");
	sprite.setTexture(bg);

	message = "Type in your name to start playing";

	text = sf::Text(message, font, 18);
	text.setFillColor(sf::Color(255, 255, 255));
	text.setStyle(sf::Text::Bold);
	text.setPosition(200, 100);

	input = "";

	nameText = sf::Text(input, font, 18);
	nameText.setFillColor(sf::Color(0, 0, 0));
	nameText.setStyle(sf::Text::Bold);
	nameText.setPosition(220, 130);

	nameRectangle = sf::RectangleShape(sf::Vector2f(400, 30));
	nameRectangle.setFillColor(sf::Color(255, 255, 255, 150));
	nameRectangle.setPosition(200, 130);
}

void Game::run()
{
	setUp(); // Setting Up the GUI
	// App loop
	while (window.isOpen())
	{
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close(); // Close windows if X is pressed 
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape)
					window.close(); // Close windows if ESC is pressed 
				if (playing) { // Manage events when playing
					// Checking Movement
					cDir.x = 0;
					cDir.y = 0;
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
						cDir.y--;
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
						cDir.y++;
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
						cDir.x--;
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
						cDir.x++;
					character.Move(cDir);
					// Managing Shooting
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
						cDir.x = 1; // Default shoot direction
						cDir.y = 0; // Default shoot direction

						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
							cDir.y = -1;
							cDir.x = 0;
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
							cDir.y = 1;
							cDir.x = 0;
						}
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
							cDir.x = -1;
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
							cDir.x = 1;
						bullets.push_back(Bullet(character.GetPos(), cDir));
					}
					break;
				}
				if (!playing) {
					// Manage events when no playing
					if ((event.key.code == sf::Keyboard::Delete || event.key.code == sf::Keyboard::BackSpace) && input.getSize() > 0) {
						input.erase(input.getSize() - 1, input.getSize());
					}
					else if (event.key.code == sf::Keyboard::Return && input.getSize() > 0) { playing = true; }
					else { input += key2str(event.key.code); }
				}
			}
		}
		window.clear();
		window.draw(sprite);


		// GIU draw when no playing
		if (!playing) {
			// When no playing
			window.draw(nameRectangle);
			nameText.setString(input);
			window.draw(nameText);
			window.draw(text);
		}
		else {
			// When playing
			window.draw(character.GetSprite());
			window.draw(character2.GetSprite());

			// Bullets update
			auto it2 = bullets.begin();
			while (it2 != bullets.end()) {

				if (character2.CheckShoot(*it2)) {
					it2 = bullets.erase(it2);
					continue;
				}
				// If out-of-bounds, the bullet is erased from the list
				if ((*it2).OutOfBounds()) {
					it2 = bullets.erase(it2);
					continue;
				}
				(*it2).Move();
				window.draw((*it2).GetShape());
				it2++;
			}

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		window.display();
	}
}
