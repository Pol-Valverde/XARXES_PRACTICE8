#include "entities.h"



Bullet::Bullet(sf::Vector2f _pos, sf::Vector2f _angle)
{
	_pos.y++; _pos.x++;
	pos = _pos;
	angle = _angle;
	bullet.setRadius(5.f);
	bullet.setFillColor(sf::Color::Red);
	bullet.setPosition(pos*SIZE);
}

void Bullet::Move()
{
	pos = pos + angle* maxSpeed;
	bullet.setPosition(pos*SIZE);
}

bool Bullet::OutOfBounds() {
	if (pos.x*SIZE < 10 || pos.y*SIZE < 10 || pos.x*SIZE > 820 || pos.y*SIZE > 560)
	{
		return true;
	}
	return false;
}

sf::Vector2f Bullet::GetPos()
{
	return pos;
}
sf::CircleShape Bullet::GetShape()
{
	return bullet;
}





Character::Character(sf::Vector2f initPos, int spriteNum)
{
	pos = initPos;
	std::string path = "resources/pj" + std::to_string(spriteNum);

	if (!characterTex.loadFromFile(path + ".png"))
	{
		std::cout << "Can't load avatar's texture" << std::endl;
	}
	sprite.setTexture(characterTex);
	sprite.setPosition(pos.x*SIZE, pos.y*SIZE);
}

sf::Vector2f Character::GetPos()
{
	return pos;
}

sf::Sprite Character::GetSprite()
{
	return sprite;
}

void Character::Move(sf::Vector2f dir)
{
	sprite.setPosition((pos.x+dir.x)*SIZE, (pos.y + dir.y)*SIZE);
	SetPos(sf::Vector2f((pos.x + dir.x), (pos.y + dir.y)));
}

void Character::SetPos(sf::Vector2f newPos)
{
	pos = newPos;
}

bool Character::OutOfBounds() {
	if (pos.x*SIZE < 10 || pos.y*SIZE < 10 || pos.x*SIZE > 820 || pos.y*SIZE > 560)
	{
		return true;
	}
	return false;
}

bool Character::CheckShoot(Bullet b) {
	sf::Vector2f bpos = b.GetPos();
	if ((bpos.x >= pos.x && bpos.x <= pos.x + 2) 
		&& (bpos.y >= pos.y && bpos.y <= pos.y + 2)
		){
		std::cout << "shoot " << life << std::endl;
		life--;
		if (life <= 0) sprite.setRotation(90);
		return true;
	}
	return false;
}
