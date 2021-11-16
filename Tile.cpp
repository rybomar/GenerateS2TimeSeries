#pragma once
#include "Tile.h"

Tile::Tile()
{
}

Tile::Tile(string name) {
	this->tileName = name;
}

string Tile::getName()
{
	return this->tileName;
}
