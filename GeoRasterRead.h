#pragma once
#include <gdal.h>
#include <gdal_priv.h>
#include <string>
#include "Types.h"

using namespace std;
class GeoRasterRead
{
	GDALDataset* dataSet;//uchwyt do pliku obrazu
	string imagePath;
	Size size;
	int bands;
	double* geoTransform;
	string projection;


public:
	GeoRasterRead();
	GeoRasterRead(string imagePath);
	~GeoRasterRead();
	string getImagePath();
	void init();
	double* getGeoTransform();
	string getProjection();
	Size getSize();
	void dispose();
	GDALDataset* getDataSet();

};

