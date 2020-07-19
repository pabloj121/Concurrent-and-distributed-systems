#include <iostream>
#include <iomanip>
#include <random>
#include "HoareMonitor.hpp"
#include <cassert>
#include <thread>
#include <mutex>
#include <future>
#include <chrono> // duraciones (duration), unidades de tiempo
//ALUMNO: PABLO MERINO AVILA 75935597-Q
using namespace std ;
using namespace HM ;

//g++ -std=c++11 -pthread -o ejecutable fumadores.cpp HoareMonitor.cpp

mutex c;
const int NUM_FUMADORES = 6;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio(){
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}
void fumar( int num_fumador ){

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
class Estanco : public HoareMonitor{
	private:
		int mostrador;
		CondVar colaEst, colaFum[NUM_FUMADORES];
 	public:
 		Estanco();
 		void ponerIngrediente(int i); //funcion de Estanquero
 		void esperarRecogidaIngrediente(); //funcion de Estanquero
 		
 		void obtenerIngrediente(int i); //funcion de fumador[i] 		
};

Estanco::Estanco(){
	mostrador= -1;
	colaEst = newCondVar();
	for(int i=0;i<NUM_FUMADORES;i++){
		colaFum[i] = newCondVar();
	}
}

void Estanco::obtenerIngrediente(int i){
	if(mostrador != i%3){ //si el ingrediente es distinto al del fumador el fumador espera;
		colaFum[i].wait();
	}
	mostrador = -1;
	colaEst.signal();
}


void Estanco::ponerIngrediente(int i){
	mostrador = i;
    int a= aleatorio<0,1>();
    if(a==0){
	    colaFum[i].signal();
    }
    else{
        colaFum[i+3].signal();
    }

}
void Estanco::esperarRecogidaIngrediente(){
	if(mostrador != -1){
		colaEst.wait();
	}
}

void funcion_hebra_estanquero(MRef<Estanco> monitor){
	int ingre=0;
	while(true){
		ingre = aleatorio<0,2>();
		
        monitor->ponerIngrediente(ingre);
		monitor->esperarRecogidaIngrediente();
        c.lock();
		cout << "--->El estanquero pone en el mostrador: " << ingre << endl;
		c.unlock();
	}
}
	

void  funcion_hebra_fumador( int num_fumador, MRef<Estanco> monitor ){
	while(true){
		monitor->obtenerIngrediente(num_fumador);
		c.lock();
		cout << "El fumador " << num_fumador << " recoge su ingrediente "  << endl;
		//c.unlock();
        //c.lock();
		fumar(num_fumador);
        c.unlock();
	}
}








//----------------------------------------------------------------------

int main(){
	//num_hebras_cita = 10 ; // número de hebras en cita
	// crear monitor (’monitor’ es una referencia al mismo, de tipo MRef<...>)
	MRef<Estanco> monitor = Create<Estanco>();
	// crear y lanzar todas las hebras (se les pasa ref. a monitor)
	
	thread fumadores[NUM_FUMADORES];
	thread hebra_estanquero;

	hebra_estanquero = thread (funcion_hebra_estanquero, monitor);

	for( unsigned i = 0 ; i < NUM_FUMADORES ; i++ )
		fumadores[i] = thread( funcion_hebra_fumador, i, monitor );
	// esperar a que terminen las hebras (no pasa nunca)

	hebra_estanquero.join();

	for( unsigned i = 0 ; i < NUM_FUMADORES ; i++ )
		fumadores[i].join();
}
