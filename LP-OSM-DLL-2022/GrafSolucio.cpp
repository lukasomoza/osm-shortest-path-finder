#include "pch.h"
#include "GrafSolucio.h"
#include "MapaSolucio.h"
#include <stack>

const double GrafSolucio::DISTMAX = 1e9;

GrafSolucio::GrafSolucio()
{
	m_numNodes = 0;
	m_numArestes = 0;
}


size_t GrafSolucio::cerca(const Coordinate& coord) const
{
	for (size_t i = 0; i < m_numNodes; i++)
	{
		if (coordenadesIguals(m_nodes[i], coord))
			return i;
	}
	return -1;
}


bool GrafSolucio::coordenadesIguals(const Coordinate& c1, const Coordinate& c2) const
{
	bool latIgual = fabs(c1.lat - c2.lat) < 0.000001;
	bool lonIgual = fabs(c1.lon - c2.lon) < 0.000001;

	return latIgual && lonIgual;
}


void GrafSolucio::afegirAresta(int posNode1, int posNode2, double pes)
{
	if (posNode1 >= 0 && posNode1 < m_numNodes && posNode2 >= 0 && posNode2 < m_numNodes && posNode1 != posNode2)
	{
		m_matriuAdj[posNode1][posNode2] = pes;
		m_matriuAdj[posNode2][posNode1] = pes;
		m_numArestes++;
	}
}


void GrafSolucio::afegirNode(const Coordinate& coord)
{
	for (int i = 0; i < m_numNodes; i++)
		if (coordenadesIguals(m_nodes[i], coord))
			return;

	m_nodes.push_back(coord);
	m_numNodes++;
}

void GrafSolucio::crearMatriu(const vector<CamiBase*>& camins)
{
	m_nodes.clear();
	m_matriuAdj.clear();
	m_numNodes = 0;
	m_numArestes = 0;
	
	for (int i = 0; i < camins.size(); i++) //recorro todos los caminos del mapa (con highway)
	{
		vector<Coordinate> coordenadesCami = camins[i]->getCamiCoords();
		for (int j = 0; j < coordenadesCami.size(); j++)  //recorro las coordenadas del camino "i"
		{
			//coordenada "j" del cami "i"
			afegirNode(coordenadesCami[j]);
		}
	}

	m_matriuAdj.resize(m_numNodes);
	for (int i = 0; i < m_numNodes; i++)
	{
		m_matriuAdj[i].resize(m_numNodes, 0);
	}


	for (int i = 0; i < camins.size(); i++)
	{
		vector<Coordinate> coordenadesCami = camins[i]->getCamiCoords();
		for (int j = 0; j < coordenadesCami.size() - 1; j++)
		{
			size_t posCoord1 = cerca(coordenadesCami[j]);
			size_t posCoord2 = cerca(coordenadesCami[j + 1]);
			
			if (posCoord1 != -1 && posCoord2 != -1 && posCoord1 != posCoord2)
			{
				Coordinate coord1 = coordenadesCami[j];
				Coordinate coord2 = coordenadesCami[j + 1];

				double distancia = Util::DistanciaHaversine(coord1, coord2);
				if (distancia > 0.0)
				{
					afegirAresta(posCoord1, posCoord2, distancia);
				}
			}
		}
	}
}


void GrafSolucio::dijkstra(size_t node, vector<double>& dist, vector<size_t>& anterior)
{
	anterior.resize(m_numNodes, 0);
	dist.resize(m_numNodes, DISTMAX);
	vector<bool> visitat(m_numNodes, false);
	dist[node] = 0.0;

	for(size_t i = 0; i < m_numNodes - 1; i++)
	{
		size_t minimaDist = minDistance(dist, visitat);   //nodo con menor distancia 
		{
			if (minimaDist != -1)   //comprobvamos si nodo existe
			{
				visitat[minimaDist] = true;   //marcamos el nodo con menor distancia como visitado
				for (size_t j = 0; j < m_numNodes; j++)
				{
					if (m_matriuAdj[minimaDist][j] != 0 && !visitat[j])     //comprobamos en la fila del nodo(minimaDist) las conexiones y si esas conexiones no estan visitadas
					{
						double distance = m_matriuAdj[minimaDist][j] + dist[minimaDist];   //calculamos la distancia de la conexion + la distancia acumulada del nodo minimaDist
						if (distance < dist[j])    //comprobamos si la distance calculada es menor que la que ya tenia el nodo "j" 
						{
							dist[j] = distance;    //la distancia al nodo "j" sera la calculada previamente 
							anterior[j] = minimaDist;   //el nodo anterior a "j" sera minimaDist
						}
					}
				}
			}
		}
	}

}


size_t GrafSolucio::minDistance(const vector<double>& dist, const vector<bool>& visitat) const
{
	double min = DISTMAX;
	size_t minIndex = -1;

	for (size_t i = 0; i < m_numNodes; i++)
	{
		if (!visitat[i] && dist[i] <= min)
		{
			min = dist[i];
			minIndex = i;
		}
	}
	return minIndex;
}


vector<Coordinate> GrafSolucio::camiMesCurt(size_t posNode1, size_t posNode2, const vector<size_t>& anterior) const
{
	std::stack<Coordinate> camiPila;
	std::vector<Coordinate> camiVector;
	
	while (posNode1 != posNode2)
	{
		camiPila.push(m_nodes[posNode2]);
		posNode2 = anterior[posNode2];
	}

	camiPila.push(m_nodes[posNode1]);

	while (!camiPila.empty())
	{
		camiVector.push_back(camiPila.top());
		camiPila.pop();
	}
	return camiVector;
}




