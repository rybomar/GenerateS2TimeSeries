#pragma once
#include "GeoRasterRead.h"
#include "Tools.h"
#include "boost/date_time/gregorian/gregorian.hpp"
class OneDateImage
{
	GeoRasterRead* imageReader;
public:
	string imagePath;
	string imageName;
	string imageDate;//YYYYMMDD
	int imageDateDOY;//Day Of Year
	boost::gregorian::date gregorianDate;
	string satelliteVersion;


	OneDateImage();
	OneDateImage(string imagePath);
	void init();
	GeoRasterRead *getReader();

	struct compareByDOY {
		bool operator()(OneDateImage const& a, OneDateImage const& b) const noexcept {
			return a.imageDateDOY < b.imageDateDOY;
		}
		bool operator()(vector<OneDateImage> const& a, vector<OneDateImage> const& b) const noexcept {
			return a[0].imageDateDOY < b[0].imageDateDOY;
		}
	};
};
