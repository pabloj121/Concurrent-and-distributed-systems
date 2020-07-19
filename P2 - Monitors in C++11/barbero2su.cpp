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

void EsperarFueraBarberia( int cliente ){
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
		CondVar barbero[2]; 
		CondVar silla[2];
		CondVar sala[N];
		int cont, vacia;
		pair<int, int> clientes_actuales[N];
		//int current_client[2];
	public:
		Barberia();
		int siguienteCliente();
		void cortarPelo(int cliente);
		void finCliente();
		bool sillasVacias();
};

Barberia::Barberia( ){
	cont = -1; vacia = 0;
	for (int i = 0; i < 2; ++i){
		clientes_actuales[i].first = clientes_actuales[i].second = 0;		
		barbero[i] = newCondVar();
		silla[i] = newCondVar();
	}
	for (int i = 0; i < N; ++i){
		sala[i] = newCondVar();
	}
}



int Barberia::sillasVacias(){
	if (silla[0].empty(){ 
		return 1;
	}
	else if(silla[1].empty())
		return 2;
	return 0;
}

void Barberia::cortarPelo( int cliente ){
	cont++;
	if(clientes_actuales)
	current_client = cliente;
	
	if(monitor->sillasVacias()){		//if ( cont > 0 ){
		sala[cliente].wait();
	}
	barbero.signal();
	silla[vacia].wait();
}

int Barberia::siguienteCliente( ){
	// vemos si hay alguna silla vacía
	if((vacia = monitor->sillasVacias()) > 0 && sala.empty()){ //if(cont < 0){ --------- // silla.empty() && sala.empty()
		barbero.wait();
	}
	else if(silla.empty()){//cont ==0
		// si no hay nadie atendido
		if(clientes_actuales[0].first == 0)
			clientes_actuales[].
		else
			clientes_actuales[1].first = 
		sala[( current_client + 1 )%N].signal(); // despertamos al siguiente cliente
	}

	return (current_client + 1 )%N;
}

void Barberia::finCliente( int silla_barberia ){
	cont--;
	silla[silla_barberia].signal(); // se va a esperar fuera
}


void funcion_hebra_barbero( MRef<Barberia> monitor, int silla_barberia ){	
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
void funcion_hebra_cliente( MRef<Barberia> monitor, int cliente ){
	while(true){
		cout << "Aqui no da core3" << endl;
		monitor->cortarPelo(cliente);
		cout << "Aqui no da core4" << endl;
		EsperarFueraBarberia(cliente);
	}
}

int main( ){
	//Fumadores monitor;
	MRef<Barberia> monitor = Create<Barberia>();
	future <void> cliente[N];
  	future <void> barbero[2];
  
	barbero = async(launch::async, funcion_hebra_barbero, monitor, int silla_barberia);
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