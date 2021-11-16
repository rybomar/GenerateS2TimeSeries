#pragma once
#include <gdal.h>
#include <gdal_priv.h>
#include "cpl_string.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "Types.h"
#include "GeoRasterRead.h"
#include "Tools.h"

class GeoRasterWrite
{
protected:
	GDALDataset* dataSet;//uchwyt do pliku obrazu
	string imagePath;
	Size size;
	int bands;
	double* geoTransform;
	string projection;
	unsigned short* fullImage;
public:
	GeoRasterWrite();
	GeoRasterWrite(string imagePath, Size size, string projection, double* geoRef);
	GeoRasterWrite(string imagePath, GeoRasterRead copyRaster);
	void save(short* fullImage);
	void save(unsigned short* fullImage);
	void save(unsigned short* imageLinesPart, int linesCount, int lineStart);
	void save(uint8_t* fullImage);
	string getImagePath();
	void dispose();
};

