#pragma once
#include "pch.h"
#include <string>
#include <vector>
#include "CamiBase.h"
using namespace std;

class GrafSolucio
{
public:
	GrafSolucio();
	~GrafSolucio() {};

	size_t cerca(const Coordinate& coord) const;

	bool coordenadesIguals(const Coordinate& c1, const Coordinate& c2) const;

	void afegirAresta(int posNode1, int posNode2, double pes);

	void afegirNode(const Coordinate& coord);

	void crearMatriu(const vector<CamiBase*>& camins);
	
	void dijkstra(size_t node, vector<double>& dist, vector<size_t>& anterior);
	
	vector<Coordinate> camiMesCurt(size_t posNode1, size_t posNode2, const vector<size_t>& anterior) const;
	
	static const double DISTMAX;

private:
	vector<Coordinate> m_nodes;
	vector<vector<double>> m_matriuAdj;
	size_t m_numNodes;
	size_t m_numArestes;

	size_t minDistance(const vector<double>& dist, const vector<bool>& visitat) const;
};
