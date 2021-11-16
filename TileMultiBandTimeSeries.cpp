#pragma once
#include "TileMultiBandTimeSeries.h"
#include <filesystem>
using namespace std;

TileMultiBandTimeSeries::TileMultiBandTimeSeries()
{
}

TileMultiBandTimeSeries::TileMultiBandTimeSeries(Tile tile, string resDir)
{
	this->tile = tile;
	this->resDir = resDir;
	this->linesPart = 1;
	this->bandNames = vector<string>();
	this->bandNames.push_back("B02");
	this->bandNames.push_back("B03");
	this->bandNames.push_back("B04");
	this->bandNames.push_back("B05");
	this->bandNames.push_back("B06");
	this->bandNames.push_back("B07");
	this->bandNames.push_back("B8A");
	this->bandNames.push_back("B11");
	this->bandNames.push_back("B12");
}

void TileMultiBandTimeSeries::CreateTimeSeries(vector<string> bandpaths02, vector<int> outputDoys)
{
	cout << "CreateTimeSeries" << endl;
	this->outputDoys = outputDoys;
	this->T = bandpaths02.size();
	for (int i = 0; i < T; i++) {
		vector< OneDateImage> oneDateImages = vector< OneDateImage>();
		//string SCLPath = this->findBandPath()
		int indexBand02InPath = bandpaths02[i].find(bandNames[0]);
		string B02 = bandpaths02[i];
		string cloudsPath = B02.replace(indexBand02InPath, bandNames[0].size(), "SCL");
		OneDateImage cimg = OneDateImage(cloudsPath);
		this->clouds.push_back(cimg);
		for (int b = 0; b < this->bandNames.size(); b++) {
			int indexBand02InPath = bandpaths02[i].find(bandNames[0]);
			string B02 = bandpaths02[i];
			string bandPath = B02.replace(indexBand02InPath, bandNames[0].size(), bandNames[b]);
			
			OneDateImage oimg = OneDateImage(bandPath);
			if (oimg.imageDateDOY >= 0 && oimg.imageDateDOY <= 366) {
				oneDateImages.push_back(oimg);
			}
		}
		this->images.push_back(oneDateImages);
	}
	std::sort(this->images.begin(), this->images.end(), OneDateImage::compareByDOY());
	this->X = this->images[0][0].getReader()->getSize().x;
	this->Y = this->images[0][0].getReader()->getSize().y;
	this->CreateEmptyStack();
	this->CreateEmptyOneTimeMatrices();
	this->ZerosOneTimeMatrices();
	this->openFilesForWrite();
}

void TileMultiBandTimeSeries::PrepareMosaics()
{
	for (int y = 0; y < Y; y += linesPart) {
		this->currentY = y;
		try {
			
			int percent = (int)((float)y / Y * 100);
			this->LoadLinesForAllBands(y);
			this->RemoveAnomalyInSeriesInPart();
		}
		catch (...) {
			cout << "Error " << y << " line";
		}
		
		for (int d = 0; d < this->outputDoys.size(); d++) {
			try {
				this->PrepareMosaicForDate(d);
			}
			catch (...) {
				cout << "Error " << d << " day";
			}
		}

	}
	for (int b = 0; b < this->bandNames.size(); b++) {
		for (int t = 0; t < this->outputDoys.size(); t++) {
			this->rastersForWrite[b][t].dispose();
		}
	}
}

void TileMultiBandTimeSeries::RemoveAnomalyInSeriesInPart()
{
	int T = this->images.size();//[t][b]
	int** pointSeriesInTime = new int*[this->bandNames.size()];
	for (int b = 0; b < this->bandNames.size(); b++) {
		pointSeriesInTime[b] = new int[T];
	}
	int* pointDiffsInTime = new int[T - 1];
	int* no0pointsSeriesInTime = new int[T];
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

	for (int y = 0; y < this->currentLinesPart; y++) {
		for (int x = 0; x < X; x++) {
			for (int b = 0; b < this->bandNames.size(); b++) {
				string bandName = this->bandNames[b];
				//cout << " " << bandName;
				if (b == 0) {//B02 - blue
					thrValueBeforeDoyThr = thrValueBeforeDoyThrBB;
					thrValueAfterDoyThr = thrValueAfterDoyThrBB;
				}
				if (b == 1) {//B03 - green
					thrValueBeforeDoyThr = thrValueBeforeDoyThrBG;
					thrValueAfterDoyThr = thrValueAfterDoyThrBG;
				}
				if (b == 2) {//B04 - red
					thrValueBeforeDoyThr = thrValueBeforeDoyThrBR;
					thrValueAfterDoyThr = thrValueAfterDoyThrBR;
				}
				if (b == 6) {//B8A - ir
					thrValueBeforeDoyThr = thrValueBeforeDoyThrBIR;
					thrValueAfterDoyThr = thrValueAfterDoyThrBIR;;
				}

		
				int count = 0;
				//loading data to temp array
				for (int t = 0; t < T; t++) {
					int value = this->multiTimeLinesPartStack[t][b][y * X + x];
					int doy = this->images[t][b].imageDateDOY;
					int thrValue = thrValueBeforeDoyThr;
					if (doy >= doyThrFilter) {
						thrValue = thrValueAfterDoyThr;
					}
					if (value > thrValue) {
						value = 0;
					}
					pointSeriesInTime[b][t] = value;
				}
				if (b > 3 && b != 6) {
					continue;
				}
				int minValue = 100000;
				int maxValue = 0;

				for (int t = 0; t < T; t++) {
					no0pointsSeriesInTime[t] = 0;
					int value = pointSeriesInTime[b][t];
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
				if (b < 3) {//only for rgb
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
				//check condition for RGB 0 values
				int i = 0;
				for (int t = 0; t < T; t++) {
					if (pointSeriesInTime[b][t] > 0) {
						pointSeriesInTime[b][t] = no0pointsSeriesInTime[i++];
					}
					if (b == 2) {
						int value0B = pointSeriesInTime[0][t];
						int value1B = pointSeriesInTime[1][t];
						int value2B = pointSeriesInTime[2][t];
						if (value0B == 0 && value1B || value0B && value2B == 0 || value1B == 0 && value2B ==0) {//if 2 bands have 0, than 3rd will be 0
							pointSeriesInTime[0][t] = 0;
							pointSeriesInTime[1][t] = 0;
							pointSeriesInTime[2][t] = 0;
						}
						else {//restore values if not 2 of rgb have 0
							pointSeriesInTime[0][t] = this->multiTimeLinesPartStack[t][0][y * X + x];
							pointSeriesInTime[1][t] = this->multiTimeLinesPartStack[t][1][y * X + x];
							pointSeriesInTime[2][t] = this->multiTimeLinesPartStack[t][2][y * X + x];
						}
					}
				}
			}
			//condition for IR and other (without rgb)
			for (int t = 0; t < T; t++) {
				if (pointSeriesInTime[6][t] == 0) {
					for (int b = 3; b < this->bandNames.size(); b++) {
						pointSeriesInTime[b][t] = 0;
					}
				}
			}
			//saving changes
			for (int t = 0; t < T; t++) {
				for (int b = 0; b < this->bandNames.size(); b++) {
					this->multiTimeLinesPartStack[t][b][y * X + x] = pointSeriesInTime[b][t];
				}
			}
		}
	}
}

void TileMultiBandTimeSeries::PrepareMosaicForDate(int indexOfDayOfYear)
{
	int dayOfYear = outputDoys[indexOfDayOfYear];
	for (int b = 0; b < this->bandNames.size(); b++) {
		vector<int> doys = vector<int>();
		int bestDoyDiff = 366;
		int bestDoy = -1;
		int indexBestDoy = -1;
		for (int i = 0; i < this->images.size(); i++) {
			int doy = this->images[i][b].imageDateDOY;
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
		for (int y = 0; y < this->currentLinesPart; y++) {
			for (int x = 0; x < X; x++) {
				int leftIndex = this->FindClosestIndexOntTheLeft(x, y, indexBestDoy, b);
				int rightIndex = this->FindClosestIndexOntTheRigt(x, y, indexBestDoy, b);
				if (leftIndex == -1 && rightIndex == -1) {
					result[y * X + x] = 0;
					continue;
				}
				if (leftIndex == -1) {
					int v = this->multiTimeLinesPartStack[rightIndex][b][y * X + x];
					result[y * X + x] = v;
					continue;
				}
				if (rightIndex == -1) {
					int v = this->multiTimeLinesPartStack[leftIndex][b][y * X + x];
					result[y * X + x] = v;
					continue;
				}
				int leftValue = this->multiTimeLinesPartStack[leftIndex][b][y * X + x];
				int rightValue = this->multiTimeLinesPartStack[rightIndex][b][y * X + x];
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
		//int bandNumberForName = this->bandNames[b];
		string bandName = Tools::getBandNameFromCreodiasPath(this->images[0][b].imagePath);
		//string date1Result = this->getResultPathAndPrepareDir("", dayOfYear, bandNumberForName);
		string date1Result = this->resDir + "\\" + this->tile.getName() + "_" + to_string(dayOfYear) + "_" + bandName + ".tif";
		GeoRasterWrite rasterWrite = this->rastersForWrite[b][indexOfDayOfYear];
		rasterWrite.save(result, this->currentLinePartEnd-this->currentLinePartStart, this->currentLinePartStart);
	}
}

void TileMultiBandTimeSeries::CreateEmptyStack()
{
	int B = this->bandNames.size();
	multiTimeLinesPartStack = new unsigned short** [T];
	for (int t = 0; t < T; t++) {
		multiTimeLinesPartStack[t] = new unsigned short*[B];
		for (int b = 0; b < B; b++) {
			int allocSize = X * this->linesPart;
			multiTimeLinesPartStack[t][b] = new unsigned short[allocSize];
			for (int xy = 0; xy < X * this->linesPart; xy++) {
				multiTimeLinesPartStack[t][b][xy] = (unsigned short)0;
			}
		}
	}
}

void TileMultiBandTimeSeries::CreateEmptyOneTimeMatrices()
{
	int allocSize16 = X * this->linesPart;
	int allocSize8 = X * this->linesPart;
	oneTimeOneBand = new short[allocSize16];
	oneTimeClouds = new uint8_t[allocSize8];
}

void TileMultiBandTimeSeries::ZerosOneTimeMatrices()
{
	for (int xy = 0; xy < X * this->linesPart; xy++) {
		oneTimeOneBand[xy] = 0;
		oneTimeClouds[xy] = 0;
	}
	result = new unsigned short[X * this->linesPart];
}

void TileMultiBandTimeSeries::LoadLinesForAllBands(int startLine)
{
	this->currentLinePartStart = startLine;
	int endLine = startLine + this->linesPart;
	if (endLine >= Y)
		endLine = Y - 1;
	this->currentLinePartEnd = endLine;
	this->currentLinesPart = endLine - startLine;
	int B = this->bandNames.size();
	int allocSize8 = X * this->linesPart;
	for (int dateIndex = 0; dateIndex < T; dateIndex++) {
		auto creader = this->clouds[dateIndex].getReader();
		GDALDataset* dataSetC = creader->getDataSet();
		GDALRasterBand* rbandC = dataSetC->GetRasterBand(1);
		rbandC->RasterIO(GF_Read, 0, startLine, X, currentLinesPart, this->oneTimeClouds, X, currentLinesPart, GDT_Byte, 0, 0);
		//GDALClose((GDALDatasetH)dataSetC);
		//delete(creader);
		for (int b = 0; b < B; b++) {
			auto bReader = this->images[dateIndex][b].getReader();
			GDALDataset* dataSet = bReader->getDataSet();
			int loadSize = X * currentLinesPart;
			GDALRasterBand* rband = dataSet->GetRasterBand(1);
			rband->RasterIO(GF_Read, 0, startLine, X, currentLinesPart, this->multiTimeLinesPartStack[dateIndex][b], X, currentLinesPart, GDT_UInt16, 0, 0);
			//GDALClose((GDALDatasetH)dataSet);
			//delete bReader;

			for (int xy = 0; xy < X * currentLinesPart; xy++) {
				//collecting data for the whole scene
				int cloudValue = this->oneTimeClouds[xy];
				if (cloudValue < 4 || cloudValue > 7) {
					this->multiTimeLinesPartStack[dateIndex][b][xy] = 0;
				}
			}
		}
	}
}

int TileMultiBandTimeSeries::FindClosestIndexOntTheLeft(int x, int y, int startIndex, int band)
{
	int foundIndex = -1;
	for (int t = startIndex; t >= 0; t--) {
		if (this->multiTimeLinesPartStack[t][band][y * X + x] > 0) {
			foundIndex = t;
			return foundIndex;
		}
	}
	return foundIndex;
}

int TileMultiBandTimeSeries::FindClosestIndexOntTheRigt(int x, int y, int startIndex, int band)
{
	int foundIndex = -1;
	for (int t = startIndex + 1; t < this->images.size(); t++) {
		if (this->multiTimeLinesPartStack[t][band][y * X + x] > 0) {
			foundIndex = t;
			return foundIndex;
		}
	}
	return foundIndex;
}

void TileMultiBandTimeSeries::openFilesForWrite()
{
	if (!boost::filesystem::exists(this->resDir)) {
		boost::filesystem::create_directories(this->resDir);
	}
	string tileResDir = this->resDir;
	if (!boost::filesystem::exists(tileResDir)) {
		boost::filesystem::create_directory(tileResDir);
	}
	int outputT = outputDoys.size();
	this->rastersForWrite = vector<vector<GeoRasterWrite>>();
	int year = this->images[0][0].gregorianDate.year();
	for (int b = 0; b < this->bandNames.size(); b++) {
		vector<GeoRasterWrite> rastersForBand = vector<GeoRasterWrite>();
		for (int t = 0; t < outputT; t++) {
			string dateResDir = tileResDir + '\\' + "day" + to_string(this->outputDoys[t]) + "_" + to_string(year);
			if (!boost::filesystem::exists(dateResDir)) {
				boost::filesystem::create_directory(dateResDir);
			}
			string resultPath = dateResDir + '\\' + this->bandNames[b] +  + ".tif";
			GeoRasterWrite rasterWrite = GeoRasterWrite(resultPath, Size(X, Y), this->images[0][0].getReader()->getProjection(), this->images[0][0].getReader()->getGeoTransform());
			rastersForBand.push_back(rasterWrite);
		}
		this->rastersForWrite.push_back(rastersForBand);//right way: [b][t]
	}
}
