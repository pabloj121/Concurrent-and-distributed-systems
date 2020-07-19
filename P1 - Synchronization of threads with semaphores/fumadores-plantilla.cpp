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

Semaphore mostrador(1);
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

void funcion_hebra_estanquero(  )
{
	int ingr;
	while(true){
	  ingr = aleatorio<0,2>();
	  wait(mostrador);
	  sem_signal(fumadores[ingr]);
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
	sem_signal(mostrador);
	fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
  future <void> hebras[3];
  future <void> estanquero;
  
  estanquero = async(launch::async, funcion_hebra_estanquero);
	 
  for(int i = 0; i < 3; i++){
	hebras[i] = async(launch::async, funcion_hebra_fumador,i);	// (modo, funcion que llamas, parametros de la funcion)
  }

  for(int i = 0; i < 3; i++){
	hebras[i].get();
  }
  estanquero.get();

}
