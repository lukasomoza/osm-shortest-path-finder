#include "pch.h"
#include "CamiSolucio.h"
#include <string>
using namespace std;


vector<Coordinate> CamiSolucio::getCamiCoords() const
{
	return m_coordenades;
}


vector<string> CamiSolucio::getReferencies() const
{
	return m_referencies;
}
