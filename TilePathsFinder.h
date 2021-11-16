#pragma once
#include <vector>
#include <iostream>
#include <boost/filesystem.hpp>
#include "Tile.h"
#include <regex>
namespace fs = std::experimental::filesystem;
using namespace std;
using namespace boost;

class TilePathsFinder
{
	boost::filesystem::path MainDir;
	string pattern10mSearch;
	string pattern20mSearch;
	string pattern10mS2B02Search;
	string patternCloudsSearch;
	string patternCloudsS2Search;
	string patternFixedCloudsSearch;

public:
	TilePathsFinder(boost::filesystem::path MainDir);

	vector<boost::filesystem::path> FindPathsAllTimesForTile(Tile searchingTile);

	boost::filesystem::path FindFilePath10mForTime(boost::filesystem::path DirPathTime);

	boost::filesystem::path FindFilePath20mForTime(boost::filesystem::path DirPathTime);

	boost::filesystem::path FindFilePathCloudsForTime(boost::filesystem::path DirPathTime);


	boost::filesystem::path FindFilePathPatternForTime(boost::filesystem::path DirPathTime, string pattern);

	vector<boost::filesystem::path> FindAllFilesPathPatternForTime(boost::filesystem::path DirPathTime, string pattern);

	vector<boost::filesystem::path> FindAllFilesPath10m(Tile tile);

	vector<boost::filesystem::path> FindAllFilesPath20m(Tile tile);

	vector<boost::filesystem::path> FindAllFilesPath10mS2B01(Tile tile);

	vector<boost::filesystem::path> FindAllFilesPathClouds(Tile tile);

	vector<boost::filesystem::path> FindAllFilesPathFixedClouds(Tile tile, boost::filesystem::path mianResDirPath);

	vector<boost::filesystem::path> FindAllFilesPathCloudsS2(Tile tile);

	boost::filesystem::path ConcatenatePathName(boost::filesystem::path DirPathTime, boost::filesystem::path name);

};
