#pragma once
#include "OneDateImage.h"

OneDateImage::OneDateImage()
{
	init();
}

OneDateImage::OneDateImage(string imagePath)
{
	init();
	this->imagePath = imagePath;
	this->imageName = Tools::fileNameFromPath(imagePath);
	this->satelliteVersion = this->imageName.substr(0, 3);
	
	//getting date
	int preDateIndex = this->imageName.find("_20");
	if (preDateIndex > 0) {
		int dateStartIndex = preDateIndex + 1;
		this->imageDate = imageName.substr(dateStartIndex, 8);
		//doy
		int year = stoi(this->imageDate.substr(0, 4));
		int month = stoi(this->imageDate.substr(4, 2));
		int day = stoi(this->imageDate.substr(6, 4));
		this->gregorianDate = boost::gregorian::date(year, month, day);
		this->imageDateDOY = this->gregorianDate.day_of_year();
	}
	else {
		preDateIndex = this->imageName.find("_Day");
		if (preDateIndex > 0) {
			int dateStartIndex = preDateIndex + 4;
			int dateEndIndex = this->imageName.find_last_of(".");
			int dateLong = dateEndIndex - dateStartIndex;
			string doyStr = this->imageName.substr(dateStartIndex, dateLong);
			int doy = stoi(doyStr);
		}
		else {
			preDateIndex = this->imageName.find("MSIL2A_2");
			if (preDateIndex > 0) {
				int dateStartIndex = preDateIndex + 8;
				this->imageDate = imageName.substr(dateStartIndex, 8);
				//doy
				int year = stoi(this->imageDate.substr(0, 4));
				int month = stoi(this->imageDate.substr(4, 2));
				int day = stoi(this->imageDate.substr(6, 4));
				this->gregorianDate = boost::gregorian::date(year, month, day);
				this->imageDateDOY = this->gregorianDate.day_of_year();
				int a = 3;
			}
		}
	}
}

void OneDateImage::init()
{
	this->imageDate = "";
	this->imageName = "";
	this->imagePath = "";
	this->imageReader = NULL;
	this->imageDateDOY = -1;
	this->satelliteVersion = "";
}

GeoRasterRead *OneDateImage::getReader()
{
	if (this->imageReader == NULL) {
		this->imageReader = new GeoRasterRead(this->imagePath);
	}
	else {
		//aa
	}

	return this->imageReader;
}
