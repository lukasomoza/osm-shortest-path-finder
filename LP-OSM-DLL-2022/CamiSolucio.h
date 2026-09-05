#pragma once
#include "CamiBase.h"
#include <string>
using namespace std;

class CamiSolucio : public CamiBase
{
public:
	CamiSolucio(vector<string> referencies, vector<Coordinate> coordenades) : m_referencies(referencies), m_coordenades(coordenades) {}
	CamiSolucio(vector<Coordinate> coordenades) : m_coordenades(coordenades) {}


	vector<Coordinate> getCamiCoords() const override;
	vector<string> getReferencies() const;

private:
	vector<string> m_referencies;
	vector<Coordinate> m_coordenades;
};

