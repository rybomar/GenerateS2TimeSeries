#pragma once
#include "Tile.h"
#include <gdal.h>
#include <boost/filesystem.hpp>
#include "TilePathsFinder.h"
#include "OneDateImage.h"
#include "GeoRasterWrite.h"
class TileTimeSeries
{
	Tile tile;
	unsigned short** multiTimeStack;


	boost::filesystem::path mainPathsWithImages;
	vector<OneDateImage> images;
	vector<OneDateImage> clouds;
	short* oneTimeOneBand;
	uint8_t* oneTimeClouds;
	bool** multiTimeClouds;
	int Y;
	int X;
	int bandNumber;
	string resDir;
	int resolution;

	int mode;

public:
	TileTimeSeries();
	TileTimeSeries(Tile tile, boost::filesystem::path mainPathsWithImages, string resDir, int bandNumber);
	void FindImagesAndPrepareVariables();
	void CreateTimeSeries();
	void CreateTimeSeries(vector<string> bandpaths, vector<string> cloudspaths);
	int getNumberOfDates();
	void PrepareMosaicForDate(int dayOfYear);
	void PrepareMosaicForDateWithDeleteAnomaly(int dayOfYear);
	void PrepareMosaicForDatePeriod(int dayOfYear, int periodDays);
	void PrepareMosaicForDatePeriodCutThr(int dayOfYear, int periodDays);
	void PrepareMosaicForDateSTD(int dayOfYear, int periodDays);
	// Saves cloud masks based on bands RGBIr (2,3,4,8) to seprated files named as 13,14,15,21
	void SaveNewCloudMasks();
	void LoadSavedCloudMasks();
	// Creates one cloud mask for one date, based on masks from 
	void SaveNewOneCloudMasks();
	void CreateTifsWithLocations();
	void RemoveAnomalyInSeries();
	void Dispose();

private:
	void CreateEmptyStack();
	void CreateEmptyOneTimeMatrices();
	void ZerosOneTimeMatrices();
	void LoadDataImageForOneDateOneBand(int dateIndex, int band);
	int FindClosestIndexOntTheLeft(int x, int y, int startIndex);
	int FindClosestIndexOntTheRigt(int x, int y, int startIndex);
	float StandardDeviation(unsigned short *array, int arraySize);
	string getResultPathAndPrepareDir(string prefix, int dayOfYear, int bandNumber);
};

