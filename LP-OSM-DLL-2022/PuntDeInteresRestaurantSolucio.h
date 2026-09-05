#pragma once
#include "PuntDeInteresBase.h"
#include <string>
using namespace std;


class PuntDeInteresRestaurantSolucio : public PuntDeInteresBase
{
public:
	PuntDeInteresRestaurantSolucio(const Coordinate& coord, const string& name, const string& tipusCuina, const string& wheelchair) :
		PuntDeInteresBase(coord, name), m_tipusCuina(tipusCuina), m_wheelchair(wheelchair) {
	}

	string getName();
	unsigned int getColor();

	string getTipusCuina() const { return m_tipusCuina; }
	string	getWheelchair() const { return m_wheelchair; }

private:
	string m_tipusCuina;
	string m_wheelchair;
};