#pragma once
#include <SFML/Graphics.hpp>
using namespace sf;

sf::View view;

void setPlayerCoordinateForViewAndIcon(float& x, float& y, Sprite& Icon, Text& health, Text& clip, Text& nick, int factor) {
	float tempXForView = x; float tempYForView = y;//считываем коорд игрока и проверяем их, чтобы убрать края
	if (x < 800) tempXForView = 800;//убираем из вида левую сторону
	if (y < 400) tempYForView = 400;//верхнюю сторону
	//if (x < 320) tempX = 320;
	//if (y < 240) tempY = 240;//верхнюю сторону
	if (y > 680) tempYForView = 680;//нижнюю стороню.для новой карты
	if (x > 1120) tempXForView = 1120;
	if (factor == 0)view.setCenter(tempXForView, tempYForView);

	Icon.setPosition(tempXForView + 600 - 300 * factor, tempYForView + 250);
	health.setPosition(tempXForView + 667 - 300 * factor, tempYForView + 295);
	clip.setPosition(tempXForView + 728 - 300 * factor, tempYForView + 295);
	nick.setPosition(tempXForView + 660 - 300 * factor, tempYForView + 255);
};