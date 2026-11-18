#pragma once
#include "MapaBase.h"
#include "PuntDeInteresBotigaSolucio.h"
#include "PuntDeInteresRestaurantSolucio.h"
#include "Util.h"
#include "BallTree.h"
#include "GrafSolucio.h"


class MapaSolucio : public MapaBase {

public:
    MapaSolucio() {
        m_ball = nullptr;
    }

    ~MapaSolucio() 
    {

    }

    void getPdis(std::vector<PuntDeInteresBase*>&);
    void getCamins(std::vector<CamiBase*>&);
    void parsejaXmlElements(std::vector<XmlElement>& xmlElements);

    CamiBase* buscaCamiMesCurt(PuntDeInteresBase* desde, PuntDeInteresBase* a);

private:
    vector<PuntDeInteresBase*> m_pdis;
    vector<CamiBase*> m_camins;
    BallTree* m_ball;
    vector<Coordinate> m_coordenadesTotsNodes;
    GrafSolucio m_graf;

    bool cercaNodeCamiGuardat(const Coordinate& coordNode);
};
