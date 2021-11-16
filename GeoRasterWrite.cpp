#pragma once
#include "GeoRasterWrite.h"

GeoRasterWrite::GeoRasterWrite()
{
}

GeoRasterWrite::GeoRasterWrite(string imagePath, Size size, string projection, double* geoRef)
{
	this->imagePath = imagePath;
	this->size = size;
	const char* pszFormat = "GTiff";
	GDALDriver* poDriver;
	char** papszMetadata;
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	char** papszOptions = NULL;
	papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
	this->dataSet = poDriver->Create(imagePath.c_str(), size.x, size.y, 1, GDT_UInt16, papszOptions);
	int xs = this->dataSet->GetRasterXSize();
	int ys = this->dataSet->GetRasterYSize();
	int zs = this->dataSet->GetRasterCount();
	if (this->dataSet != NULL) {

		//this->dataSet->SetProjection(projection.c_str());
		GDALSetProjection(dataSet, projection.c_str());
		this->dataSet->SetGeoTransform(geoRef);
	}
	unsigned int mallocSize = sizeof(short) * size.y * size.x;
	fullImage = (unsigned short*)VSIMalloc(mallocSize);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			fullImage[y * size.x + x] = 0;
		}
	}
}

GeoRasterWrite::GeoRasterWrite(string imagePath, GeoRasterRead copyRaster)
{
	const char* pszFormat = "GTiff";
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	dataSet = poDriver->CreateCopy(imagePath.c_str(), copyRaster.getDataSet(), FALSE, NULL, NULL, NULL);
	int bands = this->dataSet->GetRasterCount();
	int x = this->dataSet->GetRasterBand(1)->GetXSize();
	int y = this->dataSet->GetRasterBand(1)->GetYSize();
}

void GeoRasterWrite::save(short* fullImage)
{
	if (this->dataSet == NULL) {
		return;
	}
	GDALRasterBand* band = dataSet->GetRasterBand(1);
	int xc = this->dataSet->GetRasterBand(1)->GetXSize();
	int yc = this->dataSet->GetRasterBand(1)->GetYSize();
	unsigned int mallocSize = sizeof(short) * size.x;
	short* line = (short*)VSIMalloc(mallocSize);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			float v = fullImage[y * size.x + x];
			if (isnan(v) || isinf(v)) {
				v = 0;
			}
			line[x] = v;
		}
		band->RasterIO(GF_Write, 0, y, size.x, 1, line, size.x, 1, GDT_Int16, 0, 0);
	}
}

void GeoRasterWrite::save(unsigned short* fullImage)
{
	if (this->dataSet == NULL) {
		return;
	}
	GDALRasterBand* band = dataSet->GetRasterBand(1);
	int xc = this->dataSet->GetRasterBand(1)->GetXSize();
	int yc = this->dataSet->GetRasterBand(1)->GetYSize();
	unsigned int mallocSize = sizeof(short) * size.x;
	unsigned short* line = (unsigned  short*)VSIMalloc(mallocSize);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			unsigned short v = fullImage[y * size.x + x];
			line[x] = v;
		}
		band->RasterIO(GF_Write, 0, y, size.x, 1, line, size.x, 1, GDT_UInt16, 0, 0);
	}
}

void GeoRasterWrite::save(unsigned short* imageLinesPart, int linesCount, int lineStart)
{
	if (this->dataSet == NULL) {
		return;
	}
	GDALRasterBand* band = dataSet->GetRasterBand(1);
	int xc = this->dataSet->GetRasterBand(1)->GetXSize();
	int yc = this->dataSet->GetRasterBand(1)->GetYSize();
	unsigned int mallocSize = sizeof(short) * size.x;
	unsigned short* line = (unsigned  short*)VSIMalloc(mallocSize);
	for (int y = 0; y < linesCount; y++) {
		for (int x = 0; x < size.x; x++) {
			unsigned short v = imageLinesPart[y * size.x + x];
			line[x] = v;
		}
		band->RasterIO(GF_Write, 0, lineStart+y, size.x, 1, line, size.x, 1, GDT_UInt16, 0, 0);
	}
	VSIFree(line);
}

void GeoRasterWrite::save(uint8_t* fullImage)
{
	if (this->dataSet == NULL) {
		return;
	}
	GDALRasterBand* band = dataSet->GetRasterBand(1);
	int xc = this->dataSet->GetRasterBand(1)->GetXSize();
	int yc = this->dataSet->GetRasterBand(1)->GetYSize();
	unsigned int mallocSize = sizeof(short) * size.x;
	uint8_t* line = (uint8_t*)VSIMalloc(mallocSize);
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			float v = fullImage[y * size.x + x];
			if (isnan(v) || isinf(v)) {
				v = 0;
			}
			line[x] = v;
		}
		band->RasterIO(GF_Write, 0, y, size.x, 1, line, size.x, 1, GDT_Byte, 0, 0);
	}
}

string GeoRasterWrite::getImagePath()
{
	return imagePath;
}

void GeoRasterWrite::dispose()
{
	if (this->dataSet != NULL) {
		GDALClose((GDALDatasetH)this->dataSet);
		VSIFree(fullImage);
	}
}
