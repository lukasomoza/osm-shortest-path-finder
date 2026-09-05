#include "pch.h"
#include "BallTree.h"
#include <stack>

bool BallTree::coordenadesIguals(const Coordinate& c1, const Coordinate& c2) const
{
    bool latIgual = fabs(c1.lat - c2.lat) < 0.000001;
    bool lonIgual = fabs(c1.lon - c2.lon) < 0.000001;

    return latIgual && lonIgual;
}



void BallTree::construirArbre(const std::vector<Coordinate>& coordenades) 
{
    m_coordenades = coordenades;
    if (coordenades.size() == 1)
    {
        m_pivot = coordenades[0];
        m_radi = 0.001;
        m_left = nullptr;
        m_right = nullptr;
        return;
    }
    else
    {
        Coordinate puntCentral = Util::calcularPuntCentral(m_coordenades);
        m_pivot = puntCentral;

        int i = 0;
        double distanciaMax = 0.0;
        Coordinate A;
        while (i < m_coordenades.size())
        {
            if (Util::DistanciaHaversine(m_pivot, m_coordenades[i]) > distanciaMax)
            {
                distanciaMax = Util::DistanciaHaversine(m_pivot, m_coordenades[i]);
                A = m_coordenades[i];
            }
            i++;
        }
        m_radi = distanciaMax;

        int j = 0;
        double distMax = 0.0;
        Coordinate B = coordenades[0];
        while (j < m_coordenades.size())
        {
            if (Util::DistanciaHaversine(m_coordenades[j], A) > distMax)
            {
                distMax = Util::DistanciaHaversine(m_coordenades[j], A);
                B = m_coordenades[j];
            }
            j++;
        }

        int k = 0;
        std::vector<Coordinate> coordenadesEsq;
        std::vector<Coordinate> coordenadesDret;
        while (k < m_coordenades.size())
        {
            double distA = Util::DistanciaHaversine(m_coordenades[k], A);
            double distB = Util::DistanciaHaversine(m_coordenades[k], B);
            if (distA < distB)
            {
                coordenadesEsq.push_back(m_coordenades[k]);
            }
            else
            {
                coordenadesDret.push_back(m_coordenades[k]);
            }
            k++;
        }

        if (!coordenadesEsq.empty())
        {
                m_left = new BallTree();
                m_left->construirArbre(coordenadesEsq);
        }
        else
            m_left = nullptr;


        if (!coordenadesDret.empty())
        {
                m_right = new BallTree();
                m_right->construirArbre(coordenadesDret);
        }
        else
            m_right = nullptr;
    }
}

void BallTree::inOrdre(std::vector<std::list<Coordinate>>& out) 
{
    out.clear();
    std::stack<BallTree*> pila;

    BallTree* treeAct= this;

    while (treeAct != nullptr || !pila.empty())
    {
        while (treeAct != nullptr) 
        {
            pila.push(treeAct);
            treeAct = treeAct->m_left;
        }

        treeAct = pila.top();
        pila.pop();

        list<Coordinate> listaCoordenades;

        for (size_t i = 0; i < treeAct->m_coordenades.size(); i++)
        {
            listaCoordenades.push_back(treeAct->m_coordenades[i]);
        }
        out.push_back(listaCoordenades);
        treeAct = treeAct->m_right;
    }

}
void BallTree::preOrdre(std::vector<std::list<Coordinate>>& out) 
{
    out.clear();
    if (this == nullptr)
        return;

    stack<BallTree*> pila;

    pila.push(this);

    while (!pila.empty())
    {
        BallTree* treeAct = pila.top();
        pila.pop();
        list<Coordinate> listaCoordenades;

        for (size_t i = 0; i < treeAct->m_coordenades.size(); i++)
            listaCoordenades.push_back(treeAct->m_coordenades[i]);


        out.push_back(listaCoordenades);
     
       
        if (treeAct->m_right != nullptr)
        {
            pila.push(treeAct->m_right);
        }

        if (treeAct->m_left != nullptr)
        {
            pila.push(treeAct->m_left);
        }

    }
}

void BallTree::postOrdre(std::vector<std::list<Coordinate>>& out) {
    out.clear();
    if (this == nullptr)
        return;

    stack<BallTree*> pila1;
    stack<BallTree*> pila2;

    pila1.push(this);

    while (!pila1.empty())
    {
        BallTree* treeAct = pila1.top();
        pila1.pop();
        pila2.push(treeAct);

        if (treeAct->m_left != nullptr)
        {
            pila1.push(treeAct->m_left);
        }
        if (treeAct->m_right != nullptr)
        {
            pila1.push(treeAct->m_right);
        }
    }


    while (!pila2.empty())
    {
        BallTree* act = pila2.top();
        pila2.pop();

        list<Coordinate> listaCoordenades;
        for (size_t i = 0; i < act->m_coordenades.size(); i++)
            listaCoordenades.push_back(act->m_coordenades[i]);

        out.push_back(listaCoordenades);
    }
}

Coordinate BallTree::nodeMesProper(Coordinate targetQuery, Coordinate& Q, BallTree* ball) 
{
    double D1 = Util::DistanciaHaversine(ball->m_pivot, targetQuery);

    double D2 = Util::DistanciaHaversine(Q, targetQuery); 

    if (D1 - ball->m_radi >= D2)
        return Q;
    if (ball->isLeave())
    {
        Q = ball->m_coordenades[0];
    }
    else
    {
        double da = Util::DistanciaHaversine(ball->getEsquerre()->m_pivot, targetQuery);
        double db = Util::DistanciaHaversine(ball->getDreta()->m_pivot, targetQuery);
        if (da < db)
        {
            Q = nodeMesProper(targetQuery, Q, ball->m_left);
            Q = nodeMesProper(targetQuery, Q, ball->m_right);
        }
        else
        {
            Q = nodeMesProper(targetQuery, Q, ball->m_right);
            Q = nodeMesProper(targetQuery, Q, ball->m_left);
        }
    }
    return Q;
}






