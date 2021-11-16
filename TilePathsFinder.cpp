#pragma once
#include "TilePathsFinder.h"
#include "boost/filesystem/path.hpp"
using namespace boost::filesystem;

TilePathsFinder::TilePathsFinder(boost::filesystem::path MainDir)
{
	this->MainDir = MainDir;
    this->pattern10mSearch = "_FRE_R1.DBL.TIF";
    this->pattern20mSearch = "_FRE_R2.DBL.TIF";
    this->patternCloudsSearch = "_CLD_R1";
    this->pattern10mS2B02Search = "_B02_10m";
    this->patternCloudsS2Search = "_SCL_20m";
}

vector<boost::filesystem::path> TilePathsFinder::FindPathsAllTimesForTile(Tile searchingTile)
{
	vector<boost::filesystem::path> allPaths = vector<boost::filesystem::path>();
    //recursive_directory_iterator dir(this->MainDir), end;
    directory_iterator dir(this->MainDir), end;
    while (dir != end)
    {
        string dirName = dir->path().filename().string();
        int foundPos = dirName.find(searchingTile.getName());
        if (foundPos != string::npos)
        {
            allPaths.push_back(this->MainDir.string() + "\\" + dir->path().filename().string());
        }
        ++dir;
    }
	return allPaths;
}

boost::filesystem::path TilePathsFinder::FindFilePath10mForTime(boost::filesystem::path DirPathTime)
{
    boost::filesystem::path foundPath = boost::filesystem::path("");
    foundPath = this->FindFilePathPatternForTime(DirPathTime, this->pattern10mSearch);
    return foundPath;
}

boost::filesystem::path TilePathsFinder::FindFilePath20mForTime(boost::filesystem::path DirPathTime)
{
    boost::filesystem::path foundPath = boost::filesystem::path("");
    foundPath = this->FindFilePathPatternForTime(DirPathTime, this->pattern20mSearch);
    return foundPath;
}

boost::filesystem::path TilePathsFinder::FindFilePathCloudsForTime(boost::filesystem::path DirPathTime)
{
    boost::filesystem::path foundPath = boost::filesystem::path("");
    foundPath = this->FindFilePathPatternForTime(DirPathTime, this->patternCloudsSearch);
    return foundPath;
}

boost::filesystem::path TilePathsFinder::FindFilePathPatternForTime(boost::filesystem::path DirPathTime, string pattern)
{
    boost::filesystem::path foundPath = boost::filesystem::path("");
    vector<boost::filesystem::path> allPaths = vector<boost::filesystem::path>();
    recursive_directory_iterator dir(DirPathTime), end;
    while (dir != end)
    {
        string fileName = dir->path().filename().string();
        int foundPosPattern = fileName.find(pattern);
        int foundPosExt = fileName.find(".tif");
        int foundPosExt2 = fileName.find(".TIF");
        if (foundPosPattern != string::npos && (foundPosExt != string::npos || foundPosExt2 != string::npos))
        {
            auto aPart = dir->path().string();
            foundPath = aPart;
        }
        ++dir;
    }
    return foundPath;
}

vector<boost::filesystem::path> TilePathsFinder::FindAllFilesPathPatternForTime(boost::filesystem::path DirPathTime, string pattern)
{
    vector<boost::filesystem::path> foundPaths = vector<boost::filesystem::path>();
    boost::filesystem::path foundPath = boost::filesystem::path("");
    vector<boost::filesystem::path> allPaths = vector<boost::filesystem::path>();
    recursive_directory_iterator dir(DirPathTime), end;
    while (dir != end)
    {
        string fileName = dir->path().filename().string();
        int foundPosPattern = fileName.find(pattern);
        int foundPosExt = fileName.find(".tif");
        int foundPosExt2 = fileName.find(".TIF");
        if (foundPosPattern != string::npos && (foundPosExt != string::npos || foundPosExt2 != string::npos))
        {
            auto aPart = dir->path().string();
            foundPath = aPart;
            foundPaths.push_back(foundPath);
        }
        ++dir;
    }
    return foundPaths;
}

vector<boost::filesystem::path> TilePathsFinder::FindAllFilesPath10m(Tile tile)
{
    vector<boost::filesystem::path> all10mPaths = vector<boost::filesystem::path>();
    vector<boost::filesystem::path> dirPaths = this->FindPathsAllTimesForTile(tile);
    for (int i = 0; i < dirPaths.size(); i++) {
        boost::filesystem::path imgPath = this->FindFilePath10mForTime(dirPaths[i].string());
        if (imgPath.string() != "") {
            all10mPaths.push_back(imgPath);
        }
    }
    return all10mPaths;
}

vector<boost::filesystem::path> TilePathsFinder::FindAllFilesPath20m(Tile tile)
{
    vector<boost::filesystem::path> all20mPaths = vector<boost::filesystem::path>();
    vector<boost::filesystem::path> dirPaths = this->FindPathsAllTimesForTile(tile);
    for (int i = 0; i < dirPaths.size(); i++) {
        boost::filesystem::path imgPath = this->FindFilePath20mForTime(dirPaths[i].string());
        if (imgPath.string() != "") {
            all20mPaths.push_back(imgPath);
        }
    }
    return all20mPaths;
}

vector<boost::filesystem::path> TilePathsFinder::FindAllFilesPath10mS2B01(Tile tile)
{
    return vector<boost::filesystem::path>();
}

vector<boost::filesystem::path> TilePathsFinder::FindAllFilesPathClouds(Tile tile)
{
    vector<boost::filesystem::path> allCloudsPaths = vector<boost::filesystem::path>();
    vector<boost::filesystem::path> dirPaths = this->FindPathsAllTimesForTile(tile);
    for (int i = 0; i < dirPaths.size(); i++) {
        boost::filesystem::path imgPath = this->FindFilePathCloudsForTime(dirPaths[i].string());
        if (imgPath.string() != "") {
            allCloudsPaths.push_back(imgPath);
        }
    }
    return allCloudsPaths;
}

vector<boost::filesystem::path> TilePathsFinder::FindAllFilesPathFixedClouds(Tile tile, boost::filesystem::path mainResDirPath)
{
    vector<boost::filesystem::path> imgPaths = vector<boost::filesystem::path>();
    boost::filesystem::path tileSearchDirPath = mainResDirPath;
    for (int doy = 0; doy <= 365; doy++) {
        string searchPattern = "B14_Day" + to_string(doy) + "_Byte.tif";
        vector<boost::filesystem::path> oneDoyCldPaths = vector<boost::filesystem::path>();
//        boost::filesystem::path oneDoyPath = this->FindFilePathPatternForTime(tileSearchDirPath, searchPattern);
        oneDoyCldPaths = this->FindAllFilesPathPatternForTime(tileSearchDirPath, searchPattern);
        for (int i = 0; i < oneDoyCldPaths.size(); i++) {
            imgPaths.push_back(oneDoyCldPaths[i]);
        }
    }
    
    return imgPaths;
}

boost::filesystem::path TilePathsFinder::ConcatenatePathName(boost::filesystem::path DirPath, boost::filesystem::path name)
{
    boost::filesystem::path fullPath = DirPath.string() + "\\" + name.string();
    return fullPath;
}


