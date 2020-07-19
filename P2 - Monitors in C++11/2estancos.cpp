#include <iostream>
#include <iomanip>
#include <random>
#include <future>
#include "HoareMonitor.hpp"

using namespace std;
using namespace HM;

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

int Producir(){
	return aleatorio<0,2>();	
}

void fumar( int num_fumador){
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

class Fumadores : public HoareMonitor
{
   private:
     int      mostrador1, mostrador2;             // contador de hebras en cita, // número total de hebras en cita
     CondVar  fumadores[3], estanquero1, estanquero2;            // cola de hebras esperando en cita
   public:
     Fumadores() ; // constructor
     int obtenerIngrediente(int i);
     void ponerIngrediente(int i);
     void esperarRecogida();
};

Fumadores::Fumadores(){
    mostrador1 = mostrador2 = -1;
    for (int i = 0; i < 3; ++i){
		fumadores[i] = newCondVar();
	}
	estanquero1 = newCondVar();
    estanquero2 = newCondVar();
}

int Fumadores::obtenerIngrediente(int i){	// bien
	if(mostrador1 != i%3){ // no se ha producido el ingrediente que quiero
		fumadores[i].wait();
        mostrador1 = -1;
        estanquero1.signal();
	}
    if (mostrador2 != i%3){ // no se ha producido el ingrediente que quiero
        fumadores[i].wait();
        mostrador2 = -1;
        estanquero2.signal();
    }    
}

void Fumadores::ponerIngrediente(int i){ // bien
	most = aleatorio<1,2>();
    if(most == 1){
        mostrador1 = i;
    }
    else
        mostrador2 = i;
	fumadores[i].signal();
}

void Fumadores::esperarRecogida(){
	if(mostrador != -1){
		estanquero.wait();
	}
}

void funcion_hebra_estanquero(MRef<Fumadores> monitor){ // 1ª opcion: SU //(Fumadores *monitor) 2ª opcion: sc
	int i;
	while(true){
		i = Producir();
		monitor ->ponerIngrediente(i);
		cout << "Pone en el mostrador: " << i << endl;
			// funcionmostrador1(poner)
		//if(monitor->Mostrador() >= 0)//mostrador lleno
		monitor ->esperarRecogida();	// funcionmostrador2(esperar)
	}
}
void funcion_hebra_fumador(MRef<Fumadores> monitor, int i){
	while(true){
		monitor->obtenerIngrediente(i); // funcionmostrador3(obtener)
		cout << "El fumador recoge su ingrediente --> " << i << endl;
		fumar(i);
	}
}

int main(){
	//Fumadores monitor;
	MRef<Fumadores> monitor = Create<Fumadores>();
	future <void> fumadores[3];
  	future <void> estanquero;
  
	estanquero = async(launch::async, funcion_hebra_estanquero, monitor);
		 
	for(int i = 0; i < 3; i++){
		fumadores[i] = async(launch::async, funcion_hebra_fumador,monitor, i);	// (modo, funcion que llamas, parametros de la funcion)
	}

	for(int i = 0; i < 3; i++){
		fumadores[i].get();
	}
	estanquero.get();
}