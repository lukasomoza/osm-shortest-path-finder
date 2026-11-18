#include "pch.h"
#include "MapaSolucio.h"
#include "CamiSolucio.h"
#include "Util.h"
#include <string>
using namespace std;

void MapaSolucio::getPdis(std::vector<PuntDeInteresBase*>& pdis)
{
	pdis = m_pdis;
}


void MapaSolucio::getCamins(std::vector<CamiBase*>& camins)
{
	camins = m_camins;
}

void MapaSolucio::parsejaXmlElements(std::vector<XmlElement>& xmlElements)
{
	m_pdis.clear();
	m_camins.clear();
	m_coordenadesTotsNodes.clear();

	vector<pair<string, Coordinate>> todosLosNodos;
	vector<vector<string>> totesReferenciesAmbHighway;
	CamiBase* cami = nullptr;


	for (int i = 0; i < xmlElements.size(); i++)
	{
		string id = "";
		string lat = "";
		string lon = "";
		if (xmlElements[i].id_element == "node")
		{
			PuntDeInteresBase* pdi = nullptr;
			string name = "";
			string tipus = "";
			string estilo = "";
			string wheelchair = "";
			string openingHours = "";
			bool teName = false;
			bool esAmenity = false;

			for (int j = 0; j < xmlElements[i].atributs.size(); j++)
			{
				if (xmlElements[i].atributs[j].first == "id")
					id = xmlElements[i].atributs[j].second;

				if (xmlElements[i].atributs[j].first == "lat")
					lat = xmlElements[i].atributs[j].second;

				if (xmlElements[i].atributs[j].first == "lon")
					lon = xmlElements[i].atributs[j].second;
			}
			Coordinate coordenades;
			coordenades.lat = std::stod(lat);
			coordenades.lon = std::stod(lon);

			todosLosNodos.push_back({ id, coordenades });

			for (int j = 0; j < xmlElements[i].fills.size(); j++)
			{
				if (xmlElements[i].fills[j].first == "tag")
				{
					pair<string, string> valorTag = Util::kvDeTag(xmlElements[i].fills[j].second);

					if (valorTag.first == "name")
					{
						teName = true;
						name = valorTag.second;
					}
					else if (valorTag.first == "amenity")
						esAmenity = true;
					else if (valorTag.first == "opening_hours")
						openingHours = valorTag.second;
					else if (valorTag.first == "wheelchair")
						wheelchair = valorTag.second;
					else if (valorTag.first == "shop")
					{
						tipus = valorTag.first;
						estilo = valorTag.second;
					}
					else if (valorTag.first == "cuisine")
					{
						tipus = valorTag.first;
						estilo = valorTag.second;
					}
				}
			}
			if (teName)
			{
				if (tipus == "shop")
					pdi = new PuntDeInteresBotigaSolucio(coordenades, name, estilo, openingHours, wheelchair);
				else if (tipus == "cuisine")
					pdi = new PuntDeInteresRestaurantSolucio(coordenades, name, estilo, wheelchair);
				else if (esAmenity)
					pdi = new PuntDeInteresBase(coordenades, name);

				if (pdi != nullptr)
					m_pdis.push_back(pdi);
			}

		}

		else if (xmlElements[i].id_element == "way")
		{
			bool esHighway = false;
			vector<string> referencies;
			for (int j = 0; j < xmlElements[i].fills.size(); j++)
			{
				if (xmlElements[i].fills[j].first == "tag")
				{
					pair<string, string> valorTag = Util::kvDeTag(xmlElements[i].fills[j].second);

					if (valorTag.first == "highway")
						esHighway = true;
				}

				if (xmlElements[i].fills[j].first == "nd")
					referencies.push_back(xmlElements[i].fills[j].second[0].second);

			}


			if (esHighway)
				totesReferenciesAmbHighway.push_back(referencies);
		}
	}

	for (int i = 0; i < totesReferenciesAmbHighway.size(); i++) //recorremos todos los caminos guardados (vector de vector de strings)
	{
		vector<Coordinate> coordenades;

		for (int j = 0; j < totesReferenciesAmbHighway[i].size(); j++) //recorremos las referencias del camino "i"
		{
			int k = 0;
			while (k < todosLosNodos.size()) //recorremos todos los nodos guardados previamente 
			{
				if (totesReferenciesAmbHighway[i][j] == todosLosNodos[k].first)   // comparamos para ver si la referencia "j" del camino "i" es igual al nodo "k"
				{
					coordenades.push_back(todosLosNodos[k].second);   //comparacion = true, añadimos las coordenadas del nodo al vector de coordenadas
					if(!cercaNodeCamiGuardat(todosLosNodos[k].second))   // evita diplicados
						m_coordenadesTotsNodes.push_back(todosLosNodos[k].second);
				}
				k++;
			}
		}
		cami = new CamiSolucio(totesReferenciesAmbHighway[i], coordenades);  //creamos camino con las referencias del camino y las coordenadas 
		m_camins.push_back(cami);
	}

	if (m_ball)
	{
		delete m_ball;
		m_ball = nullptr;
	}
	m_ball = new BallTree();
	m_ball->construirArbre(m_coordenadesTotsNodes);
	m_graf.crearMatriu(m_camins);
}

bool MapaSolucio::cercaNodeCamiGuardat(const Coordinate& coordNode)
{
	bool existeix = false;
	int i = 0;
	while (i < m_coordenadesTotsNodes.size() && !existeix)
	{
		if ((fabs(m_coordenadesTotsNodes[i].lat - coordNode.lat) < 0.000001) && (fabs(m_coordenadesTotsNodes[i].lon - coordNode.lon) < 0.000001))
			existeix = true;
		i++;
	}
	return existeix;
}


CamiBase * MapaSolucio::buscaCamiMesCurt(PuntDeInteresBase *desde, PuntDeInteresBase *a) 
{
	Coordinate Q1 = Coordinate{ 0.0, 0.0 };
	Coordinate Q2 = Coordinate{ 0.0, 0.0 };

	Coordinate desdeCoord = desde->getCoord();
	Coordinate aCoord = a->getCoord();

	Coordinate nodeCamiDesde = m_ball->nodeMesProper(desdeCoord, Q1, m_ball);
	Coordinate nodeCamiA = m_ball->nodeMesProper(aCoord, Q2, m_ball);

	size_t indexDesde = m_graf.cerca(nodeCamiDesde);
	size_t indexA = m_graf.cerca(nodeCamiA);

	if (indexA == -1 || indexDesde == -1)
		return nullptr;

	vector<size_t> anterior;
	vector<double> distancies;
	
	m_graf.dijkstra(indexDesde, distancies, anterior);

	vector<Coordinate> camiSolucioCoordenades = m_graf.camiMesCurt(indexDesde, indexA, anterior);

	if (camiSolucioCoordenades.empty())
		return nullptr;

	return new CamiSolucio(camiSolucioCoordenades);
}

