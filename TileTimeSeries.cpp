#pragma once
#include "TileTimeSeries.h"
#include "OneDateImage.h"
#include "GeoRasterWrite.h"
#include <iostream>
#include <fstream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
using namespace std;

TileTimeSeries::TileTimeSeries()
{
	this->tile = Tile();
	this->multiTimeStack = nullptr;
	this->mainPathsWithImages = "";
	this->images = vector<OneDateImage>();
	this->clouds = vector<OneDateImage>();
	this->Y = 10980;
	this->X = 10980;
	this->mode = 1;
	this->resolution = 10;
}

TileTimeSeries::TileTimeSeries(Tile tile, boost::filesystem::path mainPathsWithImages, string resDir, int bandNumber)
{
	this->tile = tile;
	this->multiTimeStack = nullptr;
	this->mainPathsWithImages = mainPathsWithImages;
	this->images = vector<OneDateImage>();
	this->clouds = vector<OneDateImage>();
	this->Y = 10980/2;
	this->X = 10980/2;
	this->mode = 1;
	this->resolution = 20;
	this->bandNumber = bandNumber;
	if (bandNumber > 200) {//20m bands named as 201,202, etc.
		this->Y /= 2;
		this->X /= 2;
		this->resolution = 20;
		this->bandNumber = bandNumber - 200;
	}
	this->resDir = resDir;
}

void TileTimeSeries::FindImagesAndPrepareVariables()
{
	TilePathsFinder pathsFinder = TilePathsFinder(this->mainPathsWithImages);
	vector<boost::filesystem::path> all10mPaths = pathsFinder.FindAllFilesPath10m(this->tile);
	vector<boost::filesystem::path> all20mPaths = pathsFinder.FindAllFilesPath20m(this->tile);
	vector<boost::filesystem::path> allCloudsPaths = pathsFinder.FindAllFilesPathClouds(this->tile);
	vector<boost::filesystem::path> allFixedCloudsPaths = pathsFinder.FindAllFilesPathFixedClouds(this->tile, this->resDir);

	vector<boost::filesystem::path> allImgPaths;
	if (this->resolution == 10) {
		allImgPaths = all10mPaths;
	}
	else {
		allImgPaths = all20mPaths;
		allCloudsPaths = allFixedCloudsPaths;
	}
	for (int i = 0; i < allImgPaths.size(); i++) {
		OneDateImage oimg = OneDateImage(allImgPaths[i].string());
		OneDateImage cimg = OneDateImage(allCloudsPaths[i].string());
		if (oimg.imageDateDOY >= 0 && oimg.imageDateDOY <= 366) {
			this->images.push_back(oimg);
			this->clouds.push_back(cimg);
		}
	}
	//sorting images by dates (day of year)
	std::sort(this->images.begin(), this->images.end(), OneDateImage::compareByDOY());
	std::sort(this->clouds.begin(), this->clouds.end(), OneDateImage::compareByDOY());
	this->CreateEmptyStack();
	this->CreateEmptyOneTimeMatrices();
}

void TileTimeSeries::CreateTimeSeries()
{
	this->FindImagesAndPrepareVariables();
	cout << "DOYs: ";
	for (int i = 0; i < this->images.size(); i++) {
		this->LoadDataImageForOneDateOneBand(i, this->bandNumber);
		printf("%d, ", this->images[i].imageDateDOY);
	}
	int index = 0;
	cout << endl << endl << "Values " << index << endl;
	for (int i = 0; i < this->images.size(); i++) {
		int v = this->multiTimeStack[i][index];
		int c = this->multiTimeClouds[i][index];
		cout << i << ", " << v << ", " << c << endl;
	}

}

void TileTimeSeries::CreateTimeSeries(vector<string> allImgPaths, vector<string> allCloudsPaths)
{
	for (int i = 0; i < allImgPaths.size(); i++) {
		//for (int i = 0; i < 10; i++) {
		OneDateImage oimg = OneDateImage(allImgPaths[i]);
		OneDateImage cimg = OneDateImage(allCloudsPaths[i]);
		if (oimg.imageDateDOY >= 0 && oimg.imageDateDOY <= 366) {
			this->images.push_back(oimg);
			this->clouds.push_back(cimg);
		}
	}
	//sorting images by dates (day of year)
	std::sort(this->images.begin(), this->images.end(), OneDateImage::compareByDOY());
	std::sort(this->clouds.begin(), this->clouds.end(), OneDateImage::compareByDOY());
	this->CreateEmptyStack();
	this->CreateEmptyOneTimeMatrices();
	string bandName = Tools::getBandNameFromCreodiasPath(this->images[0].imagePath);
	this->ZerosOneTimeMatrices();

	for (int dateIndex = 0; dateIndex < allImgPaths.size(); dateIndex++) {

		GDALDataset* dataSet = this->images[dateIndex].getReader()->getDataSet();
		int loadSize = X * Y;
		GDALRasterBand* rband = dataSet->GetRasterBand(1);
		rband->RasterIO(GF_Read, 0, 0, X, Y, this->multiTimeStack[dateIndex], X, Y, GDT_UInt16, 0, 0);
		GDALClose((GDALDatasetH)dataSet);
		dataSet = this->clouds[dateIndex].getReader()->getDataSet();
		rband = dataSet->GetRasterBand(1);
		rband->RasterIO(GF_Read, 0, 0, X, Y, this->oneTimeClouds, X, Y, GDT_Byte, 0, 0);
		GDALClose((GDALDatasetH)dataSet);

		for (int xy = 0; xy < X * Y; xy++) {
			//collecting data for the whole scene
			int cloudValue = this->oneTimeClouds[xy];
			if (cloudValue < 4 || cloudValue > 7) {
				this->multiTimeStack[dateIndex][xy] = 0;
				this->multiTimeClouds[dateIndex][xy] = true;
			}
		}
	}
}

int TileTimeSeries::getNumberOfDates()
{
	return images.size();
}

void TileTimeSeries::PrepareMosaicForDate(int dayOfYear)
{
	cout << "PrepareMosaicForDate" << dayOfYear << endl;
	short* result = new short[X * Y];
	vector<int> doys = vector<int>();
	int bestDoyDiff = 366;
	int bestDoy = -1;
	int indexBestDoy = -1;
	for (int i = 0; i < this->images.size(); i++) {
		int doy = this->images[i].imageDateDOY;
		doys.push_back(doy);
		//calc the closest day of expected
		int doyDiff = std::abs(doy - dayOfYear);
		if (doyDiff < bestDoyDiff) {
			bestDoyDiff = doyDiff;
			bestDoy = doy;
			indexBestDoy = i;
		}
	}
	int newBestDoyDoyIndex = -1;
	int rest = 0;
	for (int i = 0; i < doys.size(); i++) {
		if (dayOfYear >= doys[i]) {
			newBestDoyDoyIndex = i;
			rest = dayOfYear - doys[i];
		}
	}
	indexBestDoy = newBestDoyDoyIndex;
	int indexBestBoyLeft = indexBestDoy;
	if (indexBestDoy > 0 && bestDoy > dayOfYear) {
		indexBestBoyLeft--;
	}
	for (int y=0; y<Y; y++){
		for (int x = 0; x < X; x++) {
			int leftIndex = this->FindClosestIndexOntTheLeft(x, y, indexBestDoy);
			int rightIndex = this->FindClosestIndexOntTheRigt(x, y, indexBestDoy);
			if (leftIndex == -1 && rightIndex == -1){
				result[y * X + x] = 0;
				continue;
			}
			if (leftIndex == -1) {
				int v = this->multiTimeStack[rightIndex][y * X + x];
				result[y * X + x] = v;
				continue;
			}
			if (rightIndex == -1) {
				int v = this->multiTimeStack[leftIndex][y * X + x];
				result[y * X + x] = v;
				continue;
			}
			int leftValue = this->multiTimeStack[leftIndex][y * X + x];
			int rightValue = this->multiTimeStack[rightIndex][y * X + x];
			int leftDiff = std::abs(doys[leftIndex] - dayOfYear);
			int doysRange = doys[rightIndex] - doys[leftIndex];
			float valueDiff = (float)(rightValue - leftValue);
			float valueStepInRange = 0;
			if (valueDiff != 0) {
				valueStepInRange = valueDiff / (float)doysRange;
			}
			unsigned short resValue = leftValue + leftDiff * valueStepInRange;
			result[y * X + x] = resValue;
		}
	}
	int bandNumberForName = this->bandNumber;
	string bandName = Tools::getBandNameFromCreodiasPath(this->images[0].imagePath);
	//string date1Result = this->getResultPathAndPrepareDir("", dayOfYear, bandNumberForName);
	string date1Result = this->resDir + "\\" + this->tile.getName() + "_" + to_string(dayOfYear) + "_" + bandName + ".tif";
	cout << date1Result << endl;
	GeoRasterWrite rasterWrite = GeoRasterWrite(date1Result, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
	rasterWrite.save(result);
	rasterWrite.dispose();
	
}

void TileTimeSeries::PrepareMosaicForDateWithDeleteAnomaly(int dayOfYear)
{
	short* result = new short[X * Y];
	vector<int> doys = vector<int>();

	string resultFile = this->resDir + "\\" + "days_around" + to_string(dayOfYear) + "_B" + to_string((this->bandNumber)) + ".tif";
	GeoRasterWrite rasterWrite = GeoRasterWrite(resultFile, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
	rasterWrite.save(result);
	rasterWrite.dispose();
}

void TileTimeSeries::PrepareMosaicForDatePeriod(int dayOfYear, int periodDays)
{
	short* result = new short[X * Y];
	vector<int> doys = vector<int>();
	vector<int> selectedDoyIndices = vector<int>();
	int bestDoyDiff = 366;
	int bestDoy = -1;
	int indexBestDoy = -1;
	int leftLookDays = periodDays / 2;
	int leftMinDay = dayOfYear - leftLookDays;
	int rightMaxDay = leftMinDay + periodDays;
	for (int i = 0; i < this->images.size(); i++) {
		int doy = this->images[i].imageDateDOY;
		doys.push_back(doy);
		if (doy >= leftMinDay && doy <= rightMaxDay) {
			selectedDoyIndices.push_back(i);
		}
	}
	if (this->images.size() == 0) {
		cout << "Warning: 0 images for this tile!";
		return;
	}
	vector<int> selectedValues = vector<int>(200);
	for (int y = 0; y < Y; y++) {
		for (int x = 0; x < X; x++) {
			int valuesInPeriod = 0;
			int meanValue = 0;
			int valuesSum = 0;
			int selectedValuesCount = 0;
			for (int i = 0; i < selectedDoyIndices.size(); i++) {
				int selectedIndex = selectedDoyIndices[i];
				int value = this->multiTimeStack[selectedIndex][y * X + x];
				if (value > 0) {
					selectedValues[selectedValuesCount++] = value;
				}
			}
			std::sort(selectedValues.begin(), selectedValues.begin()+selectedValuesCount);
			//cut lowest and highest values
			if (selectedValuesCount > 2) {
				for (int i = 1; i < selectedValuesCount - 1; i++) {
					valuesSum += selectedValues[i];
				}
				if (valuesSum > 0) {
					meanValue = valuesSum / (selectedValuesCount - 2);
				}
			}
			else {
				for (int i = 0; i < selectedValuesCount - 1; i++) {
					valuesSum += selectedValues[i];
				}
				if (valuesSum > 0) {
					meanValue = valuesSum / (selectedValuesCount - 1);
				}
			}
			result[y * X + x] = meanValue;
		}
	}
	//string date1Result = "F:\\MarcinRybicki\\projects\\s1gus\\S1S2\\results\\1datePeriod.tif";
	//string date1Result = "S:\\IGiK_S2\\Sen2Agri\\wielkopolskie_2018\\l2a"
	boost::filesystem::create_directory(this->resDir);
	string date1Result = this->resDir + "\\period" + to_string(periodDays) + "days_around" + to_string(dayOfYear) + "_B" + to_string((this->bandNumber)) +".tif";
	GeoRasterWrite rasterWrite = GeoRasterWrite(date1Result, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
	rasterWrite.save(result);
	rasterWrite.dispose();
}

void TileTimeSeries::PrepareMosaicForDatePeriodCutThr(int dayOfYear, int periodDays)
{
	int doyThrFilter = 100;
	/*Thresholds for filtering cloudy points*/
	int thrValueBeforeDoyThrBR = 1500;
	int thrValueAfterDoyThrBR = 2500;

	int thrValueBeforeDoyThrBG = 1000;
	int thrValueAfterDoyThrBG = 1700;

	int thrValueBeforeDoyThrBB = 1000;
	int thrValueAfterDoyThrBB = 1500;

	int thrValueBeforeDoyThrBIR = 2000;
	int thrValueAfterDoyThrBIR = 10000;

	int thrValueBeforeDoyThr = thrValueBeforeDoyThrBR;
	int thrValueAfterDoyThr = thrValueAfterDoyThrBR;

	/*switch (this->bandNumber) {
		case 
	}*/
	if (this->bandNumber == 1 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBB;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBB;
	}
	if (this->bandNumber == 2 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBG;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBG;
	}
	if (this->bandNumber == 3 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBR;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBR;
	}
	if (this->bandNumber == 4 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBIR;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBIR;;
	}

	short* result = new short[X * Y];
	vector<int> doys = vector<int>();
	vector<int> selectedDoyIndices = vector<int>();
	int bestDoyDiff = 366;
	int bestDoy = -1;
	int indexBestDoy = -1;
	int leftLookDays = periodDays / 2;
	int leftMinDay = dayOfYear - leftLookDays;
	int rightMaxDay = leftMinDay + periodDays;
	for (int i = 0; i < this->images.size(); i++) {
		int doy = this->images[i].imageDateDOY;
		doys.push_back(doy);
		if (doy >= leftMinDay && doy <= rightMaxDay) {
			selectedDoyIndices.push_back(i);
		}
	}
	if (this->images.size() == 0) {
		cout << "Warning: 0 images for this tile!";
		return;
	}
	vector<int> selectedValues = vector<int>(366);
	for (int y = 0; y < Y; y++) {
		for (int x = 0; x < X; x++) {
			int valuesInPeriod = 0;
			int meanValue = 0;
			int valuesSum = 0;
			int selectedValuesCount = 0;
			for (int i = 0; i < selectedDoyIndices.size(); i++) {
				int doy = this->images[selectedDoyIndices[i]].imageDateDOY;
				int selectedIndex = selectedDoyIndices[i];
				int value = this->multiTimeStack[selectedIndex][y * X + x];
				int thrValue = thrValueBeforeDoyThr;
				if (doy >= doyThrFilter) {
					thrValue = thrValueAfterDoyThr;
				}
				if(value < thrValue){
					selectedValues[selectedValuesCount++] = value;
				}
			}
			//cut lowest and highest values
			for (int i = 0; i < selectedValuesCount - 1; i++) {
				valuesSum += selectedValues[i];
			}
			if (valuesSum > 0) {
				meanValue = valuesSum / (selectedValuesCount - 1);
			}
			result[y * X + x] = meanValue;
		}
	}
	//string date1Result = "F:\\MarcinRybicki\\projects\\s1gus\\S1S2\\results\\1datePeriod.tif";
	//string date1Result = "S:\\IGiK_S2\\Sen2Agri\\wielkopolskie_2018\\l2a"
	boost::filesystem::create_directory(this->resDir);
	string date1Result = this->resDir + "\\period" + to_string(periodDays) + "days_around" + to_string(dayOfYear) + "_B" + to_string((this->bandNumber)) + "FltrThr" + ".tif";
	GeoRasterWrite rasterWrite = GeoRasterWrite(date1Result, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
	rasterWrite.save(result);
	rasterWrite.dispose();
}

void TileTimeSeries::PrepareMosaicForDateSTD(int dayOfYear, int periodDays)
{
	unsigned short* result = new unsigned short[X * Y];
	vector<int> doys = vector<int>();
	vector<int> selectedDoyIndices = vector<int>();
	int bestDoyDiff = 366;
	int bestDoy = -1;
	int indexBestDoy = -1;
	int leftLookDays = periodDays / 2;
	int leftMinDay = dayOfYear - leftLookDays;
	int rightMaxDay = leftMinDay + periodDays;
	for (int i = 0; i < this->images.size(); i++) {
		int doy = this->images[i].imageDateDOY;
		doys.push_back(doy);
		if (doy >= leftMinDay && doy <= rightMaxDay) {
			selectedDoyIndices.push_back(i);
		}
	}
	if (this->images.size() == 0) {
		cout << "Warning: 0 images for this tile!";
		return;
	}
	vector<int> selectedValues = vector<int>(200);
	for (int y = 0; y < Y; y++) {
		for (int x = 0; x < X; x++) {
			int valuesInPeriod = 0;
			int meanValue = 0;
			int valuesSum = 0;
			int selectedValuesCount = 0;
			for (int i = 0; i < selectedDoyIndices.size(); i++) {
				int selectedIndex = selectedDoyIndices[i];
				int value = this->multiTimeStack[selectedIndex][y * X + x];
				if (value > 0) {
					selectedValues[selectedValuesCount++] = value;
				}
			}
			
		}
	}
}

void TileTimeSeries::SaveNewCloudMasks()
{
	if (this->resolution == 20) {
		return;
	}
	for (int d = 0; d < this->images.size(); d++) {
		int dayOfYear = this->images[d].imageDateDOY;
		for (int y = 0; y < Y; y++) {
			for (int x = 0; x < X; x++) {
				this->oneTimeClouds[y * X + x] = (int)(this->multiTimeClouds[d][y * X + x]);
			}
		}
		string cloudsResFile = this->getResultPathAndPrepareDir(to_string(d), dayOfYear, 12 + this->bandNumber);
		GeoRasterWrite rasterWrite = GeoRasterWrite(cloudsResFile, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
		rasterWrite.save(this->oneTimeClouds);
		rasterWrite.dispose();
	}
}

void TileTimeSeries::LoadSavedCloudMasks()
{
	/*for (int d = 0; d < this->images.size(); d++) {
		int dayOfYear = this->images[d].imageDateDOY;
		for (int i = 0; i < 4; i++) {
			string cloudsInputFile = this->getResultPathAndPrepareDir(to_string(d), dayOfYear, 12 + this->bandNumber);
			OneDateImage cloudsInput = OneDateImage(cloudsInputFile);
			GDALDataset* dataSet = cloudsInput.getReader()->getDataSet();
			int loadSize = X * Y;
			GDALRasterBand* rband = dataSet->GetRasterBand(1);
			rband->RasterIO(GF_Read, 0, 0, X, Y, this->multiTimeClouds[d], X, Y, GDT_Byte, 0, 0);
			GDALClose((GDALDatasetH)dataSet);
		}
	}*/
}

void TileTimeSeries::SaveNewOneCloudMasks()
{
	int T = this->images.size();
	//this->LoadSavedCloudMasks();
	
	int bandsCount = 4;
	unsigned char** oneTimeBandsClouds;
	oneTimeBandsClouds = new unsigned char* [T];
	int allocSize = X * Y;
	for (int b = 0; b < bandsCount; b++) {
		oneTimeBandsClouds[b] = new unsigned char[allocSize];
	}
	for (int d = 0; d < T; d++) {
		int dayOfYear = this->images[d].imageDateDOY;
		//load previous saved masks for each bands
		for (int b = 0; b < bandsCount; b++) {
			string cloudsInputFile = this->getResultPathAndPrepareDir(to_string(d), dayOfYear, 12 + b + 1);
			OneDateImage oneDateBandImg = OneDateImage(cloudsInputFile);
			GDALDataset* dataSet = oneDateBandImg.getReader()->getDataSet();
			GDALRasterBand* rband = dataSet->GetRasterBand(1);
			rband->RasterIO(GF_Read, 0, 0, X, Y, oneTimeBandsClouds[b], X, Y, GDT_Byte, 0, 0);
			GDALClose((GDALDatasetH)dataSet);
		}
		//compare masks and mark px as masked when at least one band is marked as a cloud 
		for (int y = 0; y < Y; y++) {
			for (int x = 0; x < X; x++) {
				int sumCloudValues = 0;
				int d1pxCoord = y * X + x;
				for (int i = 0; i < bandsCount; i++) {
					sumCloudValues += oneTimeBandsClouds[i][d1pxCoord];
				}
				if (sumCloudValues > 0) {
					this->oneTimeClouds[y * X + x] = 1;
				}
				else
				{
					this->oneTimeClouds[y * X + x] = 0;
				}
			}
		}
		string cloudsOutputFile = this->getResultPathAndPrepareDir(to_string(d), dayOfYear, 100);
		GeoRasterWrite rasterWrite = GeoRasterWrite(cloudsOutputFile, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
		rasterWrite.save(this->oneTimeClouds);
		rasterWrite.dispose();
	}
}

void TileTimeSeries::CreateTifsWithLocations()
{
	short* xlocation = new short[X * Y];
	short* ylocation = new short[X * Y];
	for (int y = 0; y < Y; y++) {
		for (int x = 0; x < X; x++) {
			xlocation[y * X + x] = x;
			ylocation[y * X + x] = y;
		}
	}
	string xResult = this->resDir + "\\X.tif";
	string yResult = this->resDir + "\\Y.tif";
	GeoRasterWrite rasterWrite = GeoRasterWrite(xResult, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
	rasterWrite.save(xlocation);
	rasterWrite.dispose();
	rasterWrite = GeoRasterWrite(yResult, Size(X, Y), this->images[0].getReader()->getProjection(), this->images[0].getReader()->getGeoTransform());
	rasterWrite.save(ylocation);
	rasterWrite.dispose();
}

void TileTimeSeries::RemoveAnomalyInSeries()
{
	cout << "##DEBUG 1" << endl;
	/*if (this->resolution == 20) {
		return;
	}*/
	int T = this->images.size();
	int* pointSeriesInTime = new int[T];
	int* pointDiffsInTime = new int[T - 1];
	int* no0pointsSeriesInTime = new int[T];
	cout << "##DEBUG 1A" << endl;
	int doyThrFilter = 100;
	/*Thresholds for filtering cloudy points*/
	int thrValueBeforeDoyThrBR = 1500;
	int thrValueAfterDoyThrBR = 2500;

	int thrValueBeforeDoyThrBG = 1000;
	int thrValueAfterDoyThrBG = 1700;

	int thrValueBeforeDoyThrBB = 1000;
	int thrValueAfterDoyThrBB = 1500;

	int thrValueBeforeDoyThrBIR = 2000;
	int thrValueAfterDoyThrBIR = 10000;

	int thrValueBeforeDoyThr = 99999;
	int thrValueAfterDoyThr = 99999;

	cout << "##DEBUG 1B" << endl;

	/*switch (this->bandNumber) {
		case
	}*/
	if (this->bandNumber == 1 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBB;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBB;
	}
	if (this->bandNumber == 2 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBG;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBG;
	}
	if (this->bandNumber == 3 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBR;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBR;
	}
	if (this->bandNumber == 4 && this->resolution == 10) {
		int thrValueBeforeDoyThr = thrValueBeforeDoyThrBIR;
		int thrValueAfterDoyThr = thrValueAfterDoyThrBIR;;
	}
	cout << "##DEBUG 2" << endl;
	for (int y = 0; y < Y; y++) {
		for (int x = 0; x < X; x++) {
			int count = 0;
			//loading data to temp array
			for (int t = 0; t < T; t++) {
				int value = this->multiTimeStack[t][y * X + x];
				int doy = this->images[t].imageDateDOY;
				int thrValue = thrValueBeforeDoyThr;
				if (doy >= doyThrFilter) {
					thrValue = thrValueAfterDoyThr;
				}
				if (value > thrValue) {
					value = 0;
				}
				pointSeriesInTime[t] = value;
			}
			int minValue = 100000;
			int maxValue = 0;

			for (int t = 0; t < T; t++) {
				no0pointsSeriesInTime[t] = 0;
				int value = pointSeriesInTime[t];
				if (value > 0) {
					no0pointsSeriesInTime[count] = value;
					count++;
					if (value < minValue) {
						minValue = value;
					}
					if (value > maxValue) {
						maxValue = value;
					}
				}
			}
			int diff = maxValue - minValue;
			int step = 0;
			if (count > 0) {
				step = diff / count;
			}
			int delta_max = step * 5;

			//calc diffs
			for (int t = 0; t < count - 1; t++) {
				pointDiffsInTime[t] = no0pointsSeriesInTime[t + 1] - no0pointsSeriesInTime[t];
			}
			for (int t = 1; t < count - 2; t++) {
				if (abs(pointDiffsInTime[t]) > delta_max) {
					if (no0pointsSeriesInTime[t - 1] > no0pointsSeriesInTime[t] && no0pointsSeriesInTime[t + 1] > no0pointsSeriesInTime[t]) {//+-+
						no0pointsSeriesInTime[t] = 0;
					}
					if (no0pointsSeriesInTime[t - 1] < no0pointsSeriesInTime[t] && no0pointsSeriesInTime[t + 1] < no0pointsSeriesInTime[t]) {//-+-
						no0pointsSeriesInTime[t] = 0;
					}
				}
			}
			for (int t = 0; t < count - 1; t++) {
				if (pointDiffsInTime[t] > delta_max * 2) {
					if (t == 0 || t == T - 1) {
						no0pointsSeriesInTime[t] = 0;
					}
					else {
						if (abs(pointDiffsInTime[t - 1]) < abs(pointDiffsInTime[t + 1])) {
							no0pointsSeriesInTime[t] = 0;
						}
						else {
							no0pointsSeriesInTime[t + 1] = 0;
						}
					}
				}
			}
			//saving changes
			int i = 0;
			for (int t = 0; t < T; t++) {
				if (pointSeriesInTime[t] > 0) {
					pointSeriesInTime[t] = no0pointsSeriesInTime[i++];
				}
			}
			for (int t = 0; t < T; t++) {
				this->multiTimeStack[t][y * X + x] = pointSeriesInTime[t];
				this->multiTimeClouds[t][y * X + x] = !(bool)(pointSeriesInTime[t]);//false if no wrong data
			}
		}
	}
}

void TileTimeSeries::Dispose()
{
	int T = this->getNumberOfDates();
	for (int t = 0; t < T; t++) {
		delete multiTimeStack[t];
	}
	delete multiTimeStack;
}

void TileTimeSeries::CreateEmptyStack()
{
	int T = this->images.size();

	multiTimeStack = new unsigned short*[T];
	multiTimeClouds = new bool* [T];
	for (int t = 0; t < T; t++) {
		int allocSize = X*Y;
		multiTimeStack[t] = new unsigned short[allocSize];
		multiTimeClouds[t] = new bool[allocSize];
		for (int xy = 0; xy < X*Y; xy++) {
			multiTimeStack[t][xy] = (unsigned short)0;
			multiTimeClouds[t][xy] = false;
		}
	}

}

void TileTimeSeries::CreateEmptyOneTimeMatrices()
{
	int allocSize16 = X * Y;
	int allocSize8 = X * Y;
	oneTimeOneBand = new short[allocSize16];
	oneTimeClouds = new uint8_t[allocSize8];
	//cultivatedRaster = new double[allocSize8 * 4];
}

void TileTimeSeries::ZerosOneTimeMatrices()
{
	for (int xy = 0; xy < X * Y; xy++) {
		oneTimeOneBand[xy] = 0;
		oneTimeClouds[xy] = 0;
		//cultivatedRaster[xy] = 0;
	}
}

void TileTimeSeries::LoadDataImageForOneDateOneBand(int dateIndex, int band)
{
	this->ZerosOneTimeMatrices();

	GDALDataset *dataSet = this->images[dateIndex].getReader()->getDataSet();
	int loadSize = X*Y;
	GDALRasterBand* rband = dataSet->GetRasterBand(band);
	rband->RasterIO(GF_Read, 0, 0, X, Y, this->multiTimeStack[dateIndex], X, Y, GDT_Int16, 0, 0);
	GDALClose((GDALDatasetH)dataSet);
	dataSet = this->clouds[dateIndex].getReader()->getDataSet();
	rband = dataSet->GetRasterBand(1);
	rband->RasterIO(GF_Read, 0, 0, X, Y, this->oneTimeClouds, X, Y, GDT_Byte, 0, 0);
	GDALClose((GDALDatasetH)dataSet);

	for (int xy = 0; xy < X * Y; xy++) {
		//collecting data for the whole scene
		int cloudValue = this->oneTimeClouds[xy];
		if (cloudValue > 0) {
			this->multiTimeStack[dateIndex][xy] = 0;
			this->multiTimeClouds[dateIndex][xy] = true;
		}
	}
	
}

int TileTimeSeries::FindClosestIndexOntTheLeft(int x, int y, int startIndex)
{
	int foundIndex = -1;
	for (int t = startIndex; t >= 0; t--) {
		if (this->multiTimeStack[t][y * X + x] > 0) {
			foundIndex = t;
			return foundIndex;
		}
	}
}

int TileTimeSeries::FindClosestIndexOntTheRigt(int x, int y, int startIndex)
{
	int foundIndex = -1;
	for (int t = startIndex+1; t < this->images.size(); t++) {
		if (this->multiTimeStack[t][y * X + x] > 0) {
			foundIndex = t;
			return foundIndex;
		}
	}
	return foundIndex;
}

float TileTimeSeries::StandardDeviation(unsigned short* array, int arraySize)
{
	float sd = 0.0f;
	double var = 0;
	int sumA = 0;
	for (int n = 0; n < arraySize; n++)
	{
		sumA += array[n];
	}
	double mean = sumA / arraySize;
	for (int n = 0; n < arraySize; n++)
	{
		var += (array[n] - mean) * (array[n] - mean);
	}
	var /= arraySize;
	sd = sqrt(var);
	return sd;
}

string TileTimeSeries::getResultPathAndPrepareDir(string prefix, int dayOfYear, int bandNumber)
{
	string resSubDir = this->resDir + "\\" + to_string(dayOfYear);
	string strBname = to_string(bandNumber);
	if (bandNumber >= 13 && bandNumber < 200) {//cloud mask
		resSubDir = this->resDir + "\\cloudMasks";
		strBname = to_string(bandNumber) + "_Day" + to_string(dayOfYear);
	}
	boost::filesystem::create_directory(resSubDir);
	string pathResult = resSubDir + "\\" + prefix + "_B" + strBname + ".tif";
	return pathResult;
}
