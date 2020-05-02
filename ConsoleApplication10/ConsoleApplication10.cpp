
#include <SFML/Network.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include "server.h"
#include <mutex>
#include "ManagerOnline.h"
#include "sky.h"
inline std::mutex manager_mutex;


int main()
{

	sf::RenderWindow  window(sf::VideoMode(800, 600), "My window");
	window.setFramerateLimit(10);

	//server_forever();
	log("main", "Before run server.");
	run_daemon_server();
	log("main", "Good start server.");
	sf::Font font;
	if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
	{
		std::cout << "Can not load font." << std::endl;
		return 2;
	}


	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				break;
			}
		}

		window.clear(sf::Color::Black);

		{
			//std::lock_guard lock(manager_mutex);
			int y = 20;
			for (UserPtr user : ManagerOnline::getManager()->users) {
				sf::Text text((*user).name, font);
				text.setCharacterSize(30);
				text.setStyle(sf::Text::Bold);
				text.setFillColor(sf::Color::Red);
				text.setPosition(10, y);
				window.draw(text);
				y += 30;
				for (int fd : user->fds) {
					sf::Text text(std::to_string(fd), font);
					text.setCharacterSize(30);
					text.setStyle(sf::Text::Bold);
					text.setFillColor(sf::Color::Red);
					text.setPosition(30, y);
					window.draw(text);
					y += 30;
				}
			}
		}
		window.display();
	}
	return 0;
}

