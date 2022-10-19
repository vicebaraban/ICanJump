#include <SFML/Graphics.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>

#define WIDTH 600
#define HEIGHT 720


int random(int min, int max) {
	return min + rand() % (max - min + 1);
}



class Player
{
public:
	int size = 32;
	float speed = 0;
	int impulse = -800;
	int gravity = 1;
	int jumpTime = 0;
	int index = 0;
	sf::RectangleShape body;
	Player(std::pair<int, int> pos)
	{
		body.setSize(sf::Vector2f(size, size));
		body.setPosition(pos.first, pos.second);
		body.setFillColor(color);
	}
	void updateSpeed()
	{
		if (speed == 0 && jumpTime == 0) speed = impulse;
		else
		{
			speed += gravity * jumpTime;
			jumpTime += 1;
		}
	}
	std::pair<int, int> getPos()
	{
		int x = body.getPosition().x + size / 2;
		int y = body.getPosition().y + size / 2;
		return {x, y};
	}
private:
	sf::Color color = sf::Color(255, 0, 0);
};


class Platform
{
public:
	sf::RectangleShape body;
	int index;
	Platform(sf::Vector2f pos, sf::Vector2f size, int i)
	{
		index = i;
		body.setPosition(pos);
		body.setSize(size);
		body.setFillColor(color);
	}
private:
	sf::Color color = sf::Color(0, 255, 0);
};


class Map
{
public:
	std::vector<std::vector<int>> field;
	int lenAdditionalPart;
	int heightCompression;
	std::pair<int, int> platformSize;
	Map(std::pair<int, int> plSz, int l, int hC)
	{
		lenAdditionalPart = l;
		heightCompression = hC;
		platformSize = plSz;
		createField();
	}
	void update(int delta)
	{
		if (delta < field.size())
		{
			for (int i = 0; i < delta; i++)
			{
				field.pop_back();
				field.insert(field.begin(), createLayer());
			}
		}
	}
private:
	std::vector<int> createLayer()
	{
		std::vector<int> layer;
		for (int j = 0; j < WIDTH / platformSize.first; j++)
		{
			if (j > 0)
			{
				if (random(0, 5) == 0 && layer[j-1] != 1) layer.push_back(1);
				else layer.push_back(0);
			}
			else
			{
				if (random(0, 5) == 0) layer.push_back(1);
				else layer.push_back(0);
			}
		}
		return layer;
	}
	void createField()
	{
		for (int i = 0; i < HEIGHT / platformSize.second / heightCompression + lenAdditionalPart; i++)
		{
			field.push_back(createLayer());
		}
	}
};


int main()
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));

	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT, 32), "I Can Jump!",
		sf::Style::Titlebar | sf::Style::Close);
	window.setVerticalSyncEnabled(true);

	Player player({ 300, 566 });
	Map map({ 80, 30 }, 20, 4);
	player.index = map.field.size() - 1;
	std::vector<Platform> platforms;

	for (int i = 0; i < map.field.size(); i++)
	{
		for (int j = 0; j < map.field[0].size(); j++)
		{
			if (map.field[i][j] == 1)
			{
				platforms.push_back(Platform(sf::Vector2f(j * map.platformSize.first,
					(i - map.lenAdditionalPart) * map.platformSize.second * map.heightCompression),
					sf::Vector2f(map.platformSize.first, map.platformSize.second), i));
			}
		}
	}

	sf::Color background(0, 0, 0);

	sf::Clock clock;

	bool lose = false;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if ((event.type == sf::Event::Closed) ||
				((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
			{
				window.close();
				break;
			}

		}

		float deltaTime = clock.restart().asSeconds();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			player.body.move(-300 * deltaTime, 0);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			player.body.move(300 * deltaTime, 0);
		}

		int x = player.getPos().first;
		int y = player.body.getPosition().y;
		if (x < 0)
			player.body.setPosition(WIDTH - player.size, y);
		else if (x > WIDTH)
			player.body.setPosition(player.size, y);

		player.updateSpeed();
		player.body.move(0, player.speed * deltaTime);

		if (player.body.getPosition().y > HEIGHT)
			lose = true;

		for (int i = 0; i < platforms.size(); i++)
		{
			if (player.body.getGlobalBounds().intersects(platforms[i].body.getGlobalBounds())
				&& player.speed > 0)
			{
				int delta = abs(player.index - platforms[i].index);
				player.speed = 0;
				player.jumpTime = 0;
				player.updateSpeed();
				if (delta != 0)
				{
					map.update(delta);
					player.body.move(0, delta * map.platformSize.second * map.heightCompression);
					platforms.clear();
					for (int i = 0; i < map.field.size(); i++)
					{
						for (int j = 0; j < map.field[0].size(); j++)
						{
							if (map.field[i][j] == 1)
							{
								platforms.push_back(Platform(sf::Vector2f(j * map.platformSize.first,
									(i - map.lenAdditionalPart) * map.platformSize.second * map.heightCompression),
									sf::Vector2f(map.platformSize.first, map.platformSize.second), i));
							}
						}
					}
				}
			}
		}

		window.clear(background);
		for (int i = 0; i < platforms.size(); i++)
		{
			window.draw(platforms[i].body);
		}
		window.draw(player.body);
		window.display();

		if (lose) break;
	}

	return 0;
}