#include "pch.h"
#include "PuntDeInteresBotigaSolucio.h"

string PuntDeInteresBotigaSolucio::getName()
{
	return PuntDeInteresBase::getName();
}


unsigned int PuntDeInteresBotigaSolucio::getColor()
{


	if (m_shop == "supermarket")
		return 0xDFFF00;
	else if (m_shop == "tobacco")
		return 0xFF7F50;
	else if (m_shop == "bakery")
	{
		if (m_wheelchair == "yes" && m_openingHours.find("06:00-22:00") != -1)
			return 0x4CBB17;
		else
			return 0xFA8072;
	}

	return 0xFFEA00;

}