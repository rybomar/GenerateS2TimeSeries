#pragma once

#include <ctime>
#include "TilePathsFinder.h"
#include <gdal_priv.h>
#include <cpl_conv.h>
#include "TileMultiBandTimeSeries.h"
#include <iostream>

using namespace std;



int main(int argc, char* argv[]) {
	GDALAllRegister();
	cout << "GDALAllRegister" << "\n";
	for (int i = 0; i < argc; i++) {
		cout << argv[i] << " ";
	}
	string args = "F:/MarcinRybicki/codes/GenerateS2TimeSeries/x64/Release/33UWUtest.args";
	if (argc > 1) {
		args = argv[1];
	}
	cout << args << endl;
	//vector<string> argsFromFile = Tools::getFileContent(argv[1]);
	vector<string> argsFromFile = Tools::getFileContent(args);
	string tileName = argsFromFile[0];
	string resDir = argsFromFile[1];
	int doysCount = stoi(argsFromFile[2]);
	vector<int> outputDoys = vector<int>();
	for (int i = 3; i < 3 + doysCount; i++) {
		string sDoy = argsFromFile[i];
		int doy = stoi(sDoy);
		outputDoys.push_back(doy);
	}
	TileMultiBandTimeSeries tm = TileMultiBandTimeSeries(Tile(tileName), resDir);
	string sImgCount = argsFromFile[3 + doysCount];
	int imgCount = stoi(sImgCount);
	vector<string> bandPaths = vector<string>();
	int imgIndexStart = 3 + doysCount + 1;
	int imgIndexStop = 3 + doysCount + 1 + imgCount;
	for (int i = imgIndexStart; i < imgIndexStop; i++) {
		string path = argsFromFile[i];
		bandPaths.push_back(path);
	}
	tm.CreateTimeSeries(bandPaths, outputDoys);
	tm.PrepareMosaics();
	cout << " Done!" << endl;
	return 0;
}