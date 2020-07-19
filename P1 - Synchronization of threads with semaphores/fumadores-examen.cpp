#include <iostream>
#include <cassert>
#include <future>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

Semaphore estanqueros[2] = {0,0};
Semaphore l(0);
Semaphore fumadores[3] = {0,0,0};
// si se pidiera añadir un semáforo más, "rollo esperar a que el estanquero tenga tabaco"
//se esperaria a que esa variable se activara para "levantar" al fumador que estuviera esperando"

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(int sem, int ale){
	int ingr;
  int ultimo=3;
  bool primera=true;

	while(true){
	  ingr = aleatorio<0,2>();
    ultimo = ingr;
    if(sem!=ale || !primera){
      wait(mostrador);
    }
    else{

    }
    if(ultimo==ingr){
      cout << "El estanquero " << sem+1 << " ha producido dos veces";
      cout << " seguidas el mismo ingrediente: " << ingr << endl;
    }

  }	  
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

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
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true ){	
	sem_wait(fumadores[num_fumador]);
	cout << "Cogiendo ingrediente..." << endl;
  int sem = aleatorio<0,1>();
	sem_signal(estanqueros[sem]);
	fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
  future <void> hebras[3]; //hebras de los fumadores
  future <void> estanquero[2]; //hebra del estanquero
  int a=aleatorio<0,1>();
	
  /*if (a==0){
    cout << " EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE" << endl;
    cout << "PRIMERA HEBRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
  else{
    Semaphore r(0), s(1);
  }*/

  for(int i = 0; i < 3; i++){
	 hebras[i] = async(launch::async, funcion_hebra_fumador,i);	// (modo, funcion que llamas, parametros de la funcion)
  }
  
  for (int i = 0; i < 2; ++i){
    estanquero[i] = async(launch::async, funcion_hebra_estanquero,i);
  }

  for(int i = 0; i < 3; i++){   
    hebras[i].get();                   // .get te muestra el valor de la funcion y espera que la hebra acabe
  }                                   // los .get y async son el .join que hace que espere
 
  for (int i = 0; i <2; i++){
    estanquero[i].get(); 
      
  }
}
