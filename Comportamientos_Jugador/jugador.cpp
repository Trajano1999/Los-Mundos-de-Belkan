#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>

int tiempo_restante = 3000,						// Contador del tiempo que nos queda
	tiempo_anterior = 0,
    indicador       = 0;						// Indica la batería a recargar {0, 800, 1300, 2000, 2250}

//  ______________________________________________________________________________ 
// |                                                                              |
// |                				Funciones		   	 		                  |
// |______________________________________________________________________________|

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  	return false;
}

// Anula una matriz, esto es, la iguala a 0
void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}

// Devuelve la distancia Manhattan entre dos estados
int distanciaFinal(const estado& a, const estado& f){
	return (abs(a.fila - f.fila) + abs(a.columna - f.columna));
}

//  ______________________________________________________________________________ 
// |                                                                              |
// |                  	Métodos de ComportamientoJugador 		  	              |
// |______________________________________________________________________________|

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar
Action ComportamientoJugador::think(Sensores sensores){
	Action accion = actIDLE;
	estado destino_tmp = actual;
	tiempo_restante--;
	
	actual.fila        = sensores.posF;
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;

	destino.fila       = sensores.destinoF;
	destino.columna    = sensores.destinoC;
													
	if(!hayPlan)																					// Calculamos el camino hasta el destino si no tenemos aun un plan
		hayPlan = pathFinding(sensores.nivel, actual, destino, plan);

	if(hayPlan and plan.size() > 0){
		if(sensores.nivel == 2 and sensores.superficie[2] == 'a' and plan.front() == actFORWARD){	// En el caso en que usemos busqueda en anchura en el nivel 4, debemos esquivar a los aldeanos tambien
			if(plan.size() <= 2) 
				return actIDLE;
			else{
				destino_tmp = esquivarAldeano(sensores);
				if(destino_tmp.fila == actual.fila and destino_tmp.columna == actual.columna)		// Esto es por si no puede esquivar al aldeano
					return actIDLE;
				else{																				// El objetivo de esto es que si se choca con un aldeano espere a que se vaya
					if(tiempo_anterior != tiempo_restante + 1 ){									// y si el aldeano no se mueve en la siguiente iteracion, lo esquive
						tiempo_anterior = tiempo_restante;	
						return actIDLE;
					}else
						hayPlan = pathFinding(sensores.nivel, actual, destino_tmp, plan);
				}
			}
		}

		if(sensores.nivel == 4){
			//Actualizacion de mapa
			if(actualizarMapa(sensores))															// Si no se ha descubierto nuevo mundo, no actualizo el mapa ni la ruta
				hayPlan = pathFinding(sensores.nivel, actual, destino, plan);
			
			if( !Azapatillas and mapaResultado[sensores.posF][sensores.posC] == 'D' )
				Azapatillas = true;
			if( !Abikini and mapaResultado[sensores.posF][sensores.posC] == 'K' )
				Abikini = true;
			
			estado copia_actual = actual;															// Realizo esta copia para que HayObstaculoDelante no aumente el valor de actual
			if( HayObstaculoDelante(copia_actual) and plan.front() == actFORWARD){
				cerr << "\nRecalculando ruta ..." << endl;
				hayPlan = pathFinding(sensores.nivel, actual, destino, plan);
			}
			
			// Aldeanos
			if(sensores.superficie[2] == 'a' and plan.front() == actFORWARD){
				if(plan.size() <= 2) 
					return actIDLE;
				else{
					destino_tmp = esquivarAldeano(sensores);
					if(destino_tmp.fila == actual.fila and destino_tmp.columna == actual.columna)	// Esto es por si no puede esquivar al aldeano
						return actIDLE;
					else{																			// El objetivo de esto es que si se choca con un aldeano espere a que se vaya
						if(tiempo_anterior != tiempo_restante + 1 ){								// y si el aldeano no se mueve en la siguiente iteracion, lo esquive
							tiempo_anterior = tiempo_restante;	
							return actIDLE;
						}else
							hayPlan = pathFinding(sensores.nivel, actual, destino_tmp, plan);
					}
				}
			}

			// Bateria y Recarga
			pair<bool, estado> casilla_recarga; 
				casilla_recarga.first = false;
			
			// Mis umbrales definidos de mayor a menor
			int umbral0 = 2000,
			    umbral1 = 1000,
				umbral2 = 500;

			// Caso A3, A4, B4 
			if( (sensores.bateria > umbral0 and tiempo_restante <= umbral1) or (sensores.bateria > umbral1 and sensores.bateria <= umbral0 and tiempo_restante <= umbral2) ){
				indicador = 0;
				hayPlan = pathFinding(2, actual, destino, plan);
			}

			// Caso A1, A2
			else if(sensores.bateria > umbral0 and tiempo_restante > umbral1)
				indicador = 2250;

			// Caso C4
			else if(sensores.bateria <= umbral1 and sensores.bateria > umbral2 and tiempo_restante <= umbral2)
				indicador = 0;

			// Caso B1
			else if(sensores.bateria <= umbral0 and sensores.bateria > umbral1 and tiempo_restante > umbral0){
				casilla_recarga = busquedaRecargaEnSensores(sensores);
				if(casilla_recarga.first){
					indicador = 2250;
					hayPlan = pathFinding(sensores.nivel, actual, casilla_recarga.second, plan);
				}
			}

			// Caso B2, C2
			else if(sensores.bateria <= umbral0 and sensores.bateria > umbral2 and tiempo_restante > umbral1 and tiempo_restante <= umbral0){
				casilla_recarga = busquedaRecargaEnSensores(sensores);
				if(casilla_recarga.first){
					indicador = 2000;
					hayPlan = pathFinding(sensores.nivel, actual, casilla_recarga.second, plan);
				}
			}

			// Caso B3, C3
			else if(sensores.bateria <= umbral0 and sensores.bateria > umbral2 and tiempo_restante > umbral2 and tiempo_restante <= umbral1){
				casilla_recarga = busquedaRecargaEnSensores(sensores);
				if(casilla_recarga.first){
					indicador = 1300;
					hayPlan = pathFinding(sensores.nivel, actual, casilla_recarga.second, plan);
				}
			}

			// Caso D4
			else if(sensores.bateria <= umbral2 and tiempo_restante <= umbral2){
				casilla_recarga = busquedaRecargaEnSensores(sensores);
				if(casilla_recarga.first){
					indicador = tiempo_restante + 100;
					hayPlan = pathFinding(sensores.nivel, actual, casilla_recarga.second, plan);
				}
			}

			// Caso C1, D1
			else if(sensores.bateria <= umbral1 and tiempo_restante > umbral0){
				casilla_recarga = busquedaRecarga(sensores);
				if(casilla_recarga.first){
					indicador = 2000;
					hayPlan = pathFinding(sensores.nivel, actual, casilla_recarga.second, plan);
				}
			}

			// Caso D2
			else if(sensores.bateria <= umbral2 and tiempo_restante > umbral1 and tiempo_restante <= umbral0){
				casilla_recarga = busquedaRecarga(sensores);
				if(casilla_recarga.first){
					indicador = 1300;
					hayPlan = pathFinding(sensores.nivel, actual, casilla_recarga.second, plan);
				}
			}

			// Caso D3
			else if(sensores.bateria <= umbral2 and tiempo_restante > umbral2 and tiempo_restante <= umbral1){
				casilla_recarga = busquedaRecarga(sensores);
				if(casilla_recarga.first){
					indicador = 800;
					hayPlan = pathFinding(sensores.nivel, actual, casilla_recarga.second, plan);
				}
			}
			
			if(mapaResultado[sensores.posF][sensores.posC] == 'X' and indicador != 0 and sensores.bateria < indicador)
				return actIDLE;
		}

		accion = plan.front();															// Tomamos la siguiente accion
		plan.erase(plan.begin());														// Eliminamos la accion de la lista de acciones
	}else{
		cerr << "\nRecalculando ruta ..." << endl;
		hayPlan = pathFinding(sensores.nivel, actual, destino, plan);
	}

	return accion;
}

// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente
bool ComportamientoJugador::pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			return pathFindingProfundidad(origen,destino,plan);
			break;
		case 2: cout << "Busqueda en Anchura\n";
			return pathFindingAnchura(origen,destino,plan);
			break;
		case 3: cout << "Busqueda Costo Uniforme\n";
			return pathFindingCostoUniforme(origen,destino,plan);
			break;
		case 4: cout << "Busqueda para el reto\n";
			return pathFindingBusquedaNivel2(origen,destino,plan);
			break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}

// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  	// calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    	st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan){
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}

// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  	AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else if (*it == actTURN_L){
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}

int ComportamientoJugador::interact(Action accion, int valor){
	return false;
}

// Activa los atributos zapatillas y bikini si pasamos por una casilla de este tipo
void ComportamientoJugador::actualizarObjetos(nodo& n){
	int x = n.st.fila;
	int y = n.st.columna;

	if(!n.zapatillas and mapaResultado[x][y] == 'D')
		n.zapatillas = true;
	if(!n.bikini and mapaResultado[x][y] == 'K')
		n.bikini = true;
}

// Indica el coste que tiene cambiar de un estado a otro
int ComportamientoJugador::calcularCoste(const nodo& n){
	int x = n.st.fila,
	    y = n.st.columna,
		res = 0;

	switch (mapaResultado[x][y]){
        case 'A':
			res =  n.bikini or Abikini ? 10 : 100;
			break;
        case 'B':
			res =  n.zapatillas or Azapatillas ? 5 : 50;
			break;
        case 'T':
			res = 2;
			break;
		case '?':
			res = 3;
			break;
        default:
			res = 1;
			break;
    }

	return res;
}

// Actualiza el mapa y además indica si alguna casilla no estaba actualizada
// True si actualiza una casilla, false en caso contrario
// Además voy almacenando las casillas de recarga que me encuentre
bool ComportamientoJugador::actualizarMapa(Sensores sensores){
	estado est;
	bool actualizacion = false;
	int x       = actual.fila,
		y       = actual.columna,
		cont    = 0,
		n_giros = 0;

	if(sensores.sentido == este)  n_giros = 0;
	if(sensores.sentido == norte) n_giros = 1;
	if(sensores.sentido == oeste) n_giros = 2;
	if(sensores.sentido == sur)   n_giros = 3;

	mapaResultado[x][y] = sensores.terreno[0];

	cont = -1;
	for(int i = 1; i < 4; ++i){
		est = giro(x + cont, y + 1, n_giros);
		if(mapaResultado[est.fila][est.columna] == '?'){
			mapaResultado[est.fila][est.columna] = sensores.terreno[i];
			actualizacion = true;
		}
		if(mapaResultado[est.fila][est.columna] == 'X')
			recargas.push_back(est);
		cont++;
	}
	
	cont = -2; 
	for(int i = 4; i < 9; ++i){
		est = giro(x + cont, y + 2, n_giros);
		if(mapaResultado[est.fila][est.columna] == '?'){
			mapaResultado[est.fila][est.columna] = sensores.terreno[i];
			actualizacion = true;
		}
		if(mapaResultado[est.fila][est.columna] == 'X')
			recargas.push_back(est);
		cont++;
	}
	
	cont = -3; 
	for(int i = 9; i < 16; ++i){	
		est = giro(x + cont, y + 3, n_giros);
		if(mapaResultado[est.fila][est.columna] == '?'){
			mapaResultado[est.fila][est.columna] = sensores.terreno[i];
			actualizacion = true;
		}
		if(mapaResultado[est.fila][est.columna] == 'X')
			recargas.push_back(est);
		cont++;
	}

	return actualizacion;
}

// Función giro a los puntos a, b de 90º en sentido levógiro n veces 
estado ComportamientoJugador::giro(int a, int b, int n){
	int xres = a - actual.fila,
		yres = b - actual.columna,
		aux  = 0;
	
	for(int i = 0; i < n; ++i){
		// Aplicamos el giro de 90º
		aux = xres;
		xres = -yres;
		yres = aux;
	}
	
	estado res;
		res.fila    = actual.fila    + xres;
		res.columna = actual.columna + yres;
	
	return res; 
}

// Calcula la casilla para esquivar a un aldeano moviendose a las casillas diagonales. Si se puede ir a ambas, elige la de menor coste
estado ComportamientoJugador::esquivarAldeano(Sensores sensores){
	estado destino_tmp1 = actual,
		   destino_tmp2 = actual;
	nodo resultado1,
		 resultado2,
		 resultado;

	switch (sensores.sentido){
		case 0: 
			if(!EsObstaculo(mapaResultado[destino_tmp1.fila-1][destino_tmp1.columna-1])){ destino_tmp1.fila--; destino_tmp1.columna--; }
			if(!EsObstaculo(mapaResultado[destino_tmp2.fila-1][destino_tmp2.columna+1])){ destino_tmp2.fila--; destino_tmp2.columna++; }
			break;
		case 1: 
			if(!EsObstaculo(mapaResultado[destino_tmp1.fila-1][destino_tmp1.columna+1])){ destino_tmp1.fila--; destino_tmp1.columna++; }
			if(!EsObstaculo(mapaResultado[destino_tmp2.fila+1][destino_tmp2.columna+1])){ destino_tmp2.fila++; destino_tmp2.columna++; }
			break;
		case 2:
			if(!EsObstaculo(mapaResultado[destino_tmp1.fila+1][destino_tmp1.columna+1])){ destino_tmp1.fila++; destino_tmp1.columna++; }
			if(!EsObstaculo(mapaResultado[destino_tmp2.fila+1][destino_tmp2.columna-1])){ destino_tmp2.fila++; destino_tmp2.columna--; }
			break;
		case 3:
			if(!EsObstaculo(mapaResultado[destino_tmp1.fila+1][destino_tmp1.columna-1])){ destino_tmp1.fila++; destino_tmp1.columna--; }
			if(!EsObstaculo(mapaResultado[destino_tmp2.fila-1][destino_tmp2.columna-1])){ destino_tmp2.fila--; destino_tmp2.columna--; }
			break;
	}

	// Comparo cual es el nodo con menor coste
	resultado1.st = destino_tmp1;
	resultado2.st = destino_tmp2;

	if( ( destino_tmp1.fila != actual.fila or destino_tmp1.columna != actual.columna ) 
	and ( destino_tmp2.fila != actual.fila or destino_tmp2.columna != actual.columna ) )
		resultado = calcularCoste(resultado1) > calcularCoste(resultado2) ? resultado2 : resultado1;
	else{
		if(destino_tmp1.fila == actual.fila and destino_tmp1.columna == actual.columna)
			resultado = resultado2;
		else
			resultado = resultado1;
	}

	return resultado.st;
}

// Indica si se detecta una recarga por los sensores y donde está
// Devuelve un pair<bool, estado> indicando si hay recarga y donde
// No tiene en cuenta la posicion actual, es decir, mapaResultado[sentidos.posF][sentidos.posC]
pair<bool,estado> ComportamientoJugador::busquedaRecargaEnSensores(Sensores sensores){
	estado est,
		   res;
	int x = sensores.posF,
		y = sensores.posC,
		cont = -1, 
		n_giros;
	bool comprobar = false;
	
	if(sensores.sentido == este)  n_giros = 0;
	if(sensores.sentido == norte) n_giros = 1;
	if(sensores.sentido == oeste) n_giros = 2;
	if(sensores.sentido == sur)   n_giros = 3;

	if(!comprobar){
		for(int i = 1; i <= 3; ++i){
			est = giro(x + cont, y + 1, n_giros);
			if (mapaResultado[est.fila][est.columna] == 'X'){
				comprobar = true;
				res = est;
			}
			cont++;
		}

		if(!comprobar){
			cont = -2; 
			for(int i = 4; i < 9; ++i){
				est = giro(x + cont, y + 2, n_giros);
				if(mapaResultado[est.fila][est.columna] == 'X'){
					comprobar = true;
					res = est;
				}
				cont++;
			}

			if(!comprobar){
				cont = -3; 
				for(int i = 9; i < 16; ++i){	
					est = giro(x + cont, y + 3, n_giros);
					if(mapaResultado[est.fila][est.columna] == 'X'){
						comprobar = true;
						res = est;
					}
					cont++;
				}
			}
		}
	}

	pair<bool,estado> resultado;
	resultado.first  = comprobar;
	resultado.second = res;

	return resultado;
}

// Indica la posición de la recarga más cercana en el mapa y donde se encuentra
// Devuelve un pair<bool, estado> indicando si hay recarga y en caso afirmativo, donde está
pair<bool, estado> ComportamientoJugador::busquedaRecarga(Sensores sensores){
	estado res;
	bool comprobar = recargas.size() == 0 ? false : true;
	int minimo = mapaResultado.size();

	if(comprobar){
		for(int i=0; i<recargas.size(); ++i)
			if(distanciaFinal(actual, recargas[i]) < minimo){
				minimo = distanciaFinal(actual, recargas[i]);
				res = recargas[i];
			}
	}

	pair<bool,estado> resultado;
	resultado.first  = comprobar;
	resultado.second = res;

	return resultado;
}

//  ______________________________________________________________________________ 
// |                                                                              |
// |                		  	   	Struct	     	  	  		                  |
// |______________________________________________________________________________|

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

// Implementación del criterio de menor coste
struct CriterioCostoUniforme{
	bool operator ()(const nodo &a, const nodo &b) const{
		if (a.costeUnif > b.costeUnif)
			return true;
		else{
			if(a.costeUnif == b.costeUnif)
				ComparaEstados()(a.st,b.st);
			else
				return false;
		}
	}
};

// Implementación del criterio del Algoritmo A*
struct CriterioAestrella{
	bool operator ()(const nodo &a, const nodo &b) const{
		if (a.costeA > b.costeA)
			return true;
		else{
			if(a.costeA == b.costeA)
				ComparaEstados()(a.st,b.st);
			else
				return false;
		}
	}
};

//  ______________________________________________________________________________ 
// |                                                                              |
// |                		  Busqueda en Profundidad	     	                  |
// |______________________________________________________________________________|

// Implementación de la búsqueda en profundidad
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones
bool ComportamientoJugador::pathFindingProfundidad(const estado &origen, const estado &destino, list<Action> &plan){
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados		
	stack<nodo> pila;					  // Lista de Abiertos		

  	nodo current;
	current.st = origen;
	current.secuencia.clear();

	pila.push(current);

  	while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){
		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "\nNo encontrado plan\n";
	}

	return false;
}

//  ______________________________________________________________________________ 
// |                                                                              |
// |                			Busqueda en Anchura	   	 		                  |
// |______________________________________________________________________________|

// Implementación de la búsqueda en anchura
bool ComportamientoJugador::pathFindingAnchura(const estado &origen, const estado &destino, list<Action> &plan){
	cout << "\nCalculando plan...\n";
	plan.clear();
	
	set<estado,ComparaEstados> generados;		// Lista de Cerrados		
	queue <nodo> cola;							// Lista de Abiertos		
 	
	nodo current;
	current.st = origen;
	current.secuencia.clear();

	cola.push(current);

	while (!cola.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna)){
		cola.pop();
		generados.insert(current.st);
		
		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!cola.empty()){
			current = cola.front();
		}
	}
	
	cout << "Busqueda terminada!\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan...\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		VisualizaPlan(origen, plan);	// Ver el plan en el mapa
		return true;
	}else
		cout << "\nNo se ha encontrado plan :(\n";

	return false;
}

//  ______________________________________________________________________________ 
// |                                                                              |
// |                		 Busqueda de Costo Uniforme   		                  |
// |______________________________________________________________________________|

// Implementación de la búsqueda de costo uniforme
bool ComportamientoJugador::pathFindingCostoUniforme(const estado &origen, const estado &destino, list<Action> &plan){
	cout << "\nCalculando plan...\n";
	plan.clear();
	
	set<estado, ComparaEstados> cerrados;										// Lista de Cerrados
	priority_queue<nodo, vector<nodo>, CriterioCostoUniforme> abiertos;			// Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.clear();
	current.costeUnif = 0;

	abiertos.push(current);

	while (!abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna)){
		cerrados.insert(current.st);
		abiertos.pop();
		
		actualizarObjetos(current);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;				// Modifico la orientación
		if (cerrados.find(hijoTurnR.st) == cerrados.end()){						// Compruebo que no está en la lista de cerrados
			hijoTurnR.costeUnif += calcularCoste(current);	  					// Incremento el coste del nodo por haber cambiado de estado
			hijoTurnR.secuencia.push_back(actTURN_R);		
			abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (cerrados.find(hijoTurnL.st) == cerrados.end()){
			hijoTurnL.costeUnif += calcularCoste(current);
			hijoTurnL.secuencia.push_back(actTURN_L);
			abiertos.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (cerrados.find(hijoForward.st) == cerrados.end()){
				hijoForward.costeUnif += calcularCoste(current);
				hijoForward.secuencia.push_back(actFORWARD);
				abiertos.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la cola
		if (!abiertos.empty()){
			current = abiertos.top();											// current toma el primer valor de la cola
		}
	}

	cout << "Busqueda terminada!\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan...\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		VisualizaPlan(origen, plan);											// Ver el plan en el mapa
		return true;
	}
	else
		cout << "\nNo se ha encontrado plan :(\n";

	return false;	
}

//  ______________________________________________________________________________ 
// |                                                                              |
// |                 		 	  Algoritmo A*	    	      		              |
// |______________________________________________________________________________|

bool ComportamientoJugador::pathFindingBusquedaNivel2(const estado& origen, const estado& destino, list<Action> &plan){
	cout << "\nCalculando plan...\n";
	plan.clear();
	
	set<estado, ComparaEstados> cerrados;								// Lista de Cerrados
	priority_queue<nodo, vector<nodo>, CriterioAestrella> abiertos;		// Lista de Abiertos
	int peso = mapaResultado.size() <= 50 ? mapaResultado.size()/10 : mapaResultado.size() / 30;

	nodo current;
	current.st = origen;
	current.secuencia.clear();
	current.costeUnif = 0;
	current.costeA = distanciaFinal(current.st, destino);
	current.bikini = Abikini;
	current.zapatillas = Azapatillas;

	abiertos.push(current);
	
	while(!abiertos.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna)){
		cerrados.insert(current.st);
		abiertos.pop();
		
		actualizarObjetos(current);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;							
		if (cerrados.find(hijoTurnR.st) == cerrados.end()){
			hijoTurnR.costeUnif += calcularCoste(current);
			hijoTurnR.costeA = hijoTurnR.costeUnif + distanciaFinal(hijoTurnR.st, destino)*peso;
			hijoTurnR.secuencia.push_back(actTURN_R);		
			abiertos.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;	
		if (cerrados.find(hijoTurnL.st) == cerrados.end()){
			hijoTurnL.costeUnif += calcularCoste(current);
			hijoTurnL.costeA = hijoTurnL.costeUnif + distanciaFinal(hijoTurnL.st, destino)*peso;	
			hijoTurnL.secuencia.push_back(actTURN_L);
			abiertos.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st) and cerrados.find(hijoForward.st) == cerrados.end()){
			hijoForward.costeUnif += calcularCoste(current);
			hijoForward.costeA = hijoForward.costeUnif + distanciaFinal(hijoForward.st, destino)*peso;
			hijoForward.secuencia.push_back(actFORWARD);
			abiertos.push(hijoForward);
		}

		// Tomamos le siguiente nodo de la lista de abiertos
		if(!abiertos.empty())
			current = abiertos.top();
	}

	cout << "Busqueda terminada!\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan...\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		VisualizaPlan(origen, plan);		// Ver el plan en el mapa
		return true;
	}
	else
		cout << "\nNo se ha encontrado plan :(\n";

	return false;
}