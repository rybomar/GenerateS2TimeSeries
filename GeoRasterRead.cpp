#pragma once
#include "GeoRasterRead.h"
#include <gdal.h>
#include <gdal_priv.h>
#include <iostream>
#include <vector>
#include <fstream>
#include "Types.h"
using namespace std;

GeoRasterRead::GeoRasterRead()
{
	init();
}

GeoRasterRead::GeoRasterRead(string imagePath)
{
	init();
	this->imagePath = imagePath;
	if (ifstream(imagePath)) {
		try {
			dataSet = (GDALDataset*)GDALOpen(imagePath.c_str(), GA_ReadOnly);
			size.x = dataSet->GetRasterXSize();
			size.y = dataSet->GetRasterYSize();
			bands = dataSet->GetRasterCount();
			dataSet->GetGeoTransform(this->geoTransform);
			//projection = dataSet->GetProjectionRef();
			projection = GDALGetProjectionRef(dataSet);
		}
		catch (exception e) {
			cout << "###########";
			cout << e.what();
			int a = 3;
		}
	}
	else {
		cout << "# #ERROR while opening " << imagePath;
	}
}

GeoRasterRead::~GeoRasterRead()
{
	this->dispose();
	delete geoTransform;
}

string GeoRasterRead::getImagePath()
{
	return imagePath;
}

void GeoRasterRead::init()
{
	geoTransform = (double*)CPLMalloc(sizeof(double) * 6);
	projection = "";
}

double* GeoRasterRead::getGeoTransform()
{
	return geoTransform;
}

string GeoRasterRead::getProjection()
{
	return projection;
}

Size GeoRasterRead::getSize()
{
	return size;
}

void GeoRasterRead::dispose()
{
	if (this->dataSet != NULL) {
		//2
		GDALClose((GDALDatasetH)this->dataSet);
	}
}

GDALDataset* GeoRasterRead::getDataSet()
{
	return this->dataSet;
}