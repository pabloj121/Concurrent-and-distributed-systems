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
void CortarPeloAlCliente(){//( int num_fumador){
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_pelar( aleatorio<20,200>() );
   // informa de que comienza a fumar
    cout << "Cliente pelándose, vaya pelos traes  :";
         cout << " empieza a pelar (" << duracion_pelar.count() << " milisegundos)" << endl;
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_pelar );
   // informa de que ha terminado de fumar
   cout << "Cliente pelado, ahora espera a que te crezca el pelo..." << endl;
}

void EsperarFueraBarberia(int cliente){
	chrono::milliseconds duracion_espera( aleatorio<20,200>() );
   	this_thread::sleep_for( duracion_espera );
   	cout << "Cliente " << cliente << " ,ya puedes volver a entrar" << endl;
}

const int N = 5; // número fijo de clientes

class Barberia : public HoareMonitor
{
	private:
		// las CondVar pueden utilizar la funcion empty() de tipo
		//bool (ademas es una funcion privada)
		CondVar barbero; 
		CondVar silla;
		CondVar sala[N];
		int cont;
		int current_client;
	public:
		Barberia();
		void siguienteCliente();
		void cortarPelo(int cliente);
		void finCliente();
};

Barberia::Barberia(){
	current_client = 0;
	cont = -1;
	barbero = newCondVar();
	silla = newCondVar();
	for (int i = 0; i < N; ++i){
		sala[i] = newCondVar();
	}
}

void  Barberia::cortarPelo(int cliente){
	cont++;
	current_client = cliente;
	if ( cont > 0 ){
		sala[cliente].wait();
	}
	barbero.signal();
	//pelar();
	silla.wait();
}

/*void  Barberia::CortarPelo(){
	if(silla != 0){	//si hay alguien pelandose
		sala.wait();
	}
	else if(sala != 0){
		silla.signal();
	}
	else{
		barbero.signal();
	}
	si hay alguien en sala

	sino, pasa a la sala
}*/

void Barberia::siguienteCliente(){
	if(cont < 0){// silla.empty() && sala.empty()
		barbero.wait();
	}
	else if(silla.empty()){//cont ==0
		sala[(current_client+1)%N].signal();
	}
	/*if(cont < 0){ // si cont = 0 ya hay uno en la silla
		barbero.wait();
	}
	else if(silla == 0){
		sala.signal();
	}*/	
}

void Barberia::finCliente(){
	cont--;
	silla.signal(); // se va a esperar fuera
}

// cuando lo han pelao espera fuera pa volver a entrar
/*void Barberia::EsperarFueraBarberia(int cliente){
	//sala.wait();
   	this_thread::sleep_for( duracion_pelar );
	chrono::milliseconds duracion_pelar( aleatorio<20,200>() );
	cout << "Fin de la espera" << endl;
}*/

void funcion_hebra_barbero(MRef<Barberia> monitor){	
	while(true){
		monitor->siguienteCliente();
				cout << "Aqui no da core1" << endl;

		//monitor->CortarPeloAlCliente();
		CortarPeloAlCliente();
				cout << "Aqui no da core2" << endl;

		monitor->finCliente();
	}

}

// 0 < cliente <= N
void funcion_hebra_cliente(MRef<Barberia> monitor, int cliente){
	while(true){
		cout << "Aqui no da core3" << endl;
		monitor->cortarPelo(cliente);
		cout << "Aqui no da core4" << endl;
		EsperarFueraBarberia(cliente);
	}
}
int main(){
	//Fumadores monitor;
	MRef<Barberia> monitor = Create<Barberia>();
	future <void> cliente[N];
  	future <void> barbero;
  
	barbero = async(launch::async, funcion_hebra_barbero, monitor);
	for (int i = 0; i < N; ++i){
		cliente[i] = async(launch::async, funcion_hebra_cliente, monitor, i);
	}
	
	// aunque esperemos a que las hebras finalicen, nunca van a
	// terminar ...(while(true))	 
	for (int i = 0; i < N; ++i){
		cliente[i].get();
	}
	barbero.get();
}