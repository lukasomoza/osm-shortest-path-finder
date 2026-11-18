#pragma once
#include "PuntDeInteresBase.h"
#include <string>
using namespace std;

class PuntDeInteresBotigaSolucio : public PuntDeInteresBase
{
public:
	PuntDeInteresBotigaSolucio(const Coordinate& coord, const string& name, const string& shop, const string& openingHours, const string& wheelchair) :
		PuntDeInteresBase(coord, name), m_shop(shop), m_openingHours(openingHours), m_wheelchair(wheelchair) {
	}


	string getName();
	unsigned int getColor();

	string getShop() const { return m_shop; }
	string getOpeningHours() const { return m_openingHours; }
	string potWheelchair() const { return m_wheelchair; }

private:
	string m_shop;
	string m_openingHours;
	string m_wheelchair;
};
