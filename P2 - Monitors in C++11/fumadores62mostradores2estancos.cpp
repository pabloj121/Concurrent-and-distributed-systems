#include <iostream>
#include <cassert>
#include <thread>
#include <future>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.hpp"

using namespace std ;
using namespace HM ;

const int NUM_FUMADORES = 6;

//mutex c;

//6 FUMADORES Y 2 ESTANCOS CON 1 MOSTRADOR
//**********************************************************************
// El estanco tendrá dos estanqueros y 6 monitores.
// Dos necesitarán el ingrediente 0, otros 2 el 1 y los otros 2 el 2.
//
//----------------------------------------------------------------------

template< int min, int max > int aleatorio(){

  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

class Estanco : public HoareMonitor{
private:
  int ingrediente;
  CondVar 
    ingredientes[NUM_FUMADORES],
    mostrador_libre;
public:
  Estanco();
  int obtenerIngrediente(int f);
  int ponerIngrediente1(int i);
  int ponerIngrediente2(int i);
  void esperarRecogidaIngrediente();
  void fumar(int i);
};

Estanco::Estanco(){
  ingrediente = -1;
  for(int i = 0; i < NUM_FUMADORES; i++)
    ingredientes[i] = newCondVar();

  mostrador_libre = newCondVar();
}

int Estanco::obtenerIngrediente(int f){
  //Esperamos a que esté el ingrediente de cada fumador.
  if(ingrediente != f)
      ingredientes[f].wait();
    ingrediente = -1;
    cout << "El fumador " << f << " retira su ingrediente." << endl;
    mostrador_libre.signal();
}

int Estanco::ponerIngrediente1(int ingr){
  //Se pone el ingrediente en el mostrador.
  //variable de estado.

  cout << "Estanquero 1 produce ingediente: " << ingr << endl;
  if(ingr == 0){
    int ingre = aleatorio<0,1>();
    if(ingre == 0){
      //ingredientes[ingre].signal();
      ingrediente = ingre;
    }
    else{
      //ingredientes[1].signal();
      ingrediente = 1;
    }
  }

  if(ingr == 1){
    int ingre = aleatorio<2,3>();
    if(ingre == 2){
      //ingredientes[ingre].signal();
      ingrediente = ingre;
    }
    else{
      //ingredientes[3].signal();
      ingrediente = 3;
    }
  }

  if(ingr == 2){
    int ingre = aleatorio<4,5>();
    if(ingre == 4){
      //ingredientes[ingre].signal();
      ingrediente = ingre;
    }
    else{
      //ingredientes[5].signal();
      ingrediente = 5;
    }
  }
    ingredientes[ingrediente].signal();
}

int Estanco::ponerIngrediente2(int ingr){
  //Se pone el ingrediente en el mostrador.
  //variable de estado.

  cout << "Estanquero 2 produce ingediente: " << ingr << endl;
  if(ingr == 0){
    int ingre = aleatorio<0,1>();
    if(ingre == 0){
      //ingredientes[ingre].signal();
      ingrediente = ingre;
    }
    else{
      //ingredientes[1].signal();
      ingrediente = 1;
    }
  }

  if(ingr == 1){
    int ingre = aleatorio<2,3>();
    if(ingre == 2){
      //ingredientes[ingre].signal();
      ingrediente = ingre;
    }
    else{
      //ingredientes[3].signal();
      ingrediente = 3;
    }
  }

  if(ingr == 2){
    int ingre = aleatorio<4,5>();
    if(ingre == 4){
      //ingredientes[ingre].signal();
      ingrediente = ingre;
    }
    else{
      //ingredientes[5].signal();
      ingrediente = 5;
    }
  }
    ingredientes[ingrediente].signal();
}

void Estanco::esperarRecogidaIngrediente(){
  //Espera hasta que el mostrador está libre.
  //variable de estado.
  if(ingrediente != -1)
    mostrador_libre.wait();
  //Mostrador libre
  //Que ingrdiente
}


void funcion_hebra_estanquero1(MRef<Estanco> monitor){
	while(true){
		int ingr = aleatorio<0,2>();
		monitor->ponerIngrediente1(ingr);
    //c.lock();
		//cout << "Estanquero 1 produce ingediente: " << ingr << endl;
    //c.unlock();
    monitor->esperarRecogidaIngrediente();		
	}
}

void funcion_hebra_estanquero2(MRef<Estanco> monitor){
  while(true){
    int ingr = aleatorio<0,2>();
    monitor->ponerIngrediente2(ingr);
    monitor->esperarRecogidaIngrediente();
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void Estanco::fumar(int num_fumador){
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );
   // informa de que comienza a fumar
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for(duracion_fumar);
   // informa de que ha terminado de fumar
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador(MRef<Estanco> monitor, int num_fumador){
   while( true ){

   	monitor->obtenerIngrediente(num_fumador);
    //c.lock();
   	//cout << "El fumador " << num_fumador << " retira su ingrediente." << endl;
    //c.unlock();
   	monitor->fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main(){
  MRef<Estanco> monitor = Create<Estanco>();
	future<void> futuros[NUM_FUMADORES];
	for(int i = 0; i < NUM_FUMADORES; i++)
		futuros[i] = async(launch::async, funcion_hebra_fumador, monitor, i);

	thread hebra_estanquero1(funcion_hebra_estanquero1, monitor);
  thread hebra_estanquero2(funcion_hebra_estanquero2, monitor);

	for(int i = 0; i < NUM_FUMADORES; i++)
		futuros[i].get();
	hebra_estanquero1.join();
	hebra_estanquero2.join();
}