#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado{
    int fila;
    int columna;
    int orientacion;
};

struct nodo{
	estado st;
	list<Action> secuencia;
    int costeUnif,
        costeA;
    bool bikini     = false,
         zapatillas = false;
};

class ComportamientoJugador : public Comportamiento{
public: 
    ComportamientoJugador(unsigned int size) : Comportamiento(size){
		// Inicializar Variables de Estado
		fil = col = 99;
		brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
		destino.fila = -1;
		destino.columna = -1;
		destino.orientacion = -1;
		hayPlan = false;
    }

	ComportamientoJugador(std::vector<std::vector<unsigned char>> mapaR) : Comportamiento(mapaR){
		// Inicializar Variables de Estado
		fil = col = 99;
		brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
		destino.fila = -1;
		destino.columna = -1;
		destino.orientacion = -1;
		hayPlan = false;
    }

    ComportamientoJugador(const ComportamientoJugador &comport) : Comportamiento(comport) {}
    ~ComportamientoJugador() {}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador *clone() { return new ComportamientoJugador(*this); }

private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino;
    list<Action> plan;
    Action ultimaAccion;
    bool hayPlan,
         Azapatillas = false,
         Abikini     = false;
    vector<estado> recargas;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFindingProfundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFindingAnchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFindingCostoUniforme(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFindingBusquedaNivel2(const estado& origen, const estado& destino, list<Action> &plan);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);
    void actualizarObjetos(nodo& n);
	int calcularCoste(const nodo& n);

    // Mapa
    bool actualizarMapa(Sensores sensores);
    estado giro(int a, int b, int n);
    // Aldeanos
    estado esquivarAldeano(Sensores sensores);   
    // Recarga              
    pair<bool,estado> busquedaRecargaEnSensores(Sensores sensores);   
    pair<bool,estado> busquedaRecarga(Sensores sensores);
};

#endif
