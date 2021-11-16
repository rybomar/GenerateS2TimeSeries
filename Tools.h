#pragma once

#include <windows.h>
#include <string>
#include <complex>
#include <iostream>
#include <boost/filesystem.hpp>

using namespace std;

static class Tools {
public:
	static bool dirExists(const std::string& dirName_in)
	{
		string d = dirName_in.c_str();
		DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path!

		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;   // this is a directory!

		return false;    // this is not a directory!
	}

	static bool fileExist(string filePath) {
		int len;
		int slength = (int)filePath.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, filePath.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, filePath.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;

		LPCTSTR path = r.c_str();
		if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(path))
		{
			return false;
		}
		return true;
	}

	static void dispDebug(string s) {
		//cout << s << endl;
	}
	static void dispDebug(int i) {
		dispDebug(to_string(i));
	}
	static void dispDebug(double i) {
		dispDebug(to_string(i));
	}
	void dispDebug(float i) {
		dispDebug(to_string(i));
	}

	void dispInfoLine(string s) {
		cout << endl << s;
	}
	void dispInfoLine(int i) {
		dispInfoLine(to_string(i));
	}
	void dispInfoLine(double i) {
		dispInfoLine(to_string(i));
	}
	void dispInfoLine(float i) {
		dispInfoLine(to_string(i));
	}

	void dispInfo(string s) {
		cout << s;
	}
	void dispInfo(int i) {
		dispInfo(to_string(i));
	}
	void dispInfo(double i) {
		dispInfo(to_string(i));
	}
	void dispInfo(float i) {
		dispInfo(to_string(i));
	}

	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}

	int createDirectoryRecursively(string spath)
	{
		for (int i = 0; i < spath.size(); i++) {
			char c = spath[i];
			if (c == '\\' || c == '/' || i == spath.size() - 1) {
				string dir = spath.substr(0, i + 1);
				if (!(dirExists(dir))) {
					std::wstring stemp = s2ws(dir);
					LPCWSTR path = stemp.c_str();
					bool res = CreateDirectory(path, NULL);
				}
			}
		}
		if (dirExists(spath)) {
			return 1;
		}
		return 0;
	}

	static string fileNameFromPath(string path) {
		boost::filesystem::path filePath = path;
		auto autoName = filePath.filename();
		string strName = autoName.string();
		strName = strName.substr(0, strName.size() - 5);
		return strName;
	}

	static string getBandNameFromCreodiasPath(string path) {
		string bandName = "";
		if (path.size() > 182) {
			bandName = path.substr(179, 3);
		}
		return bandName;
	}

	static std::vector<std::string> getFileContent(std::string fileName)
	{
		std::vector<std::string> vecOfStrLines = std::vector<std::string>();
		// Open the File
		std::ifstream in(fileName.c_str());
		// Check if object is valid
		if (!in)
		{
			std::cerr << "Cannot open the File : " << fileName << std::endl;
			return vecOfStrLines;
		}
		std::string str;
		// Read the next line from File untill it reaches the end.
		while (std::getline(in, str))
		{
			// Line contains string of length > 0 then save it in vector
			if (str.size() > 0)
				vecOfStrLines.push_back(str);
		}
		//Close The File
		in.close();
		return vecOfStrLines;
	}
};