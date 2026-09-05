#pragma once
#include "pch.h"

#include "Common.h"
#include <vector>
#include <string>
using namespace std;

class CamiBase 
{
	public:
		virtual std::vector<Coordinate> getCamiCoords() const = 0;
};

