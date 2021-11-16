#pragma once

class Point {
public:
	int x, y;
	Point() {
		x = y = 0;
	}
	Point(int xPosition, int yPosition) {
		this->x = xPosition;
		this->y = yPosition;
	}
	bool equals(Point* other) {
		if (this->x == other->x && this->y == other->y)
			return true;
		return false;
	}
};

class Size : public Point {
public:
	Size() {
		x = y = 0;
	}
	Size(int width, int height) {
		this->x = width;
		this->y = height;
	}
};
