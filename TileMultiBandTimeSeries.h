#pragma once
#include "Tile.h"
#include <gdal.h>
#include <boost/filesystem.hpp>
#include "TilePathsFinder.h"
#include "OneDateImage.h"
#include "GeoRasterWrite.h"
class TileMultiBandTimeSeries
{
	Tile tile;
	unsigned short*** multiTimeLinesPartStack;
	unsigned short** oneTimeLinesPartClouds;
	unsigned short* result;
	vector<vector<OneDateImage>> images;
	vector<OneDateImage> clouds;
	string resDir;
	int linesPart;
	int currentLinePartStart, currentLinePartEnd, currentLinesPart, currentY;
	int X, Y, T;
	vector<string> bandNames;
	short* oneTimeOneBand;
	uint8_t* oneTimeClouds;
	vector<int> outputDoys;
	vector<vector<GeoRasterWrite>> rastersForWrite;

public:
	TileMultiBandTimeSeries();
	TileMultiBandTimeSeries(Tile tile, string resDir);
	void CreateTimeSeries(vector<string> bandpaths02, vector<int> outputDoys);
	void PrepareMosaics();
	void RemoveAnomalyInSeriesInPart();
	void PrepareMosaicForDate(int indexOfDayOfYear);
private:
	void CreateEmptyStack();
	void CreateEmptyOneTimeMatrices();
	void ZerosOneTimeMatrices();
	void LoadLinesForAllBands(int startLine);
	int FindClosestIndexOntTheLeft(int x, int y, int startIndex, int band);
	int FindClosestIndexOntTheRigt(int x, int y, int startIndex, int band);
	void openFilesForWrite();
};

