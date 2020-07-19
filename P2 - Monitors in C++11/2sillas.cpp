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
   cout << "Cliente pelado" << endl;
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
		CondVar barbero1, barbero2; 
		CondVar silla1, silla2;
		CondVar sala;
		int cont1, cont2;
	public:
		Barberia();
		void siguienteCliente();
		void cortarPelo(int cliente);
		void finCliente(int silla);
};

Barberia::Barberia(){
	cont1 = cont2 = -1;
    sala = newCondVar();
    barbero1 = newCondVar();
    barbero2 = newCondVar();
    silla1 = newCondVar();
    silla2 = newCondVar();
}

void Barberia::siguienteCliente(){
    if(silla1.empty() && sala.empty()){
        barbero1.wait();
    }
    else if(silla1.empty()){
        sala.signal();
    }

    if (silla2.empty() && sala.empty()){
        barbero2.wait();
    }
    else if (silla2.empty()){
        sala.signal();
    }
}

void Barberia::cortarPelo(int cliente){
    int vacia;

    if(silla1.empty())
        vacia = 1;
    else if(silla2.empty())
        vacia = 2;
    else  vacia = 0;

    if(vacia == 1){
        cont1++;
        /*if(cont1 > 0){
            sala.wait();
        }*/
        barbero1.signal();
        silla1.wait();
    }
    else if(vacia == 2){
        cont2++;
        /*if(cont2 > 0){
            sala.wait();
        }*/
        barbero2.signal();
        silla2.wait();
    }
    else
        sala.wait();

}

void Barberia::finCliente(int silla){
    if(silla==1){// silla !=0
        cont1--;
        silla1.signal();
        cout << "fin de cliente" << endl;
    }
    else{
        cont2--;
        silla2.signal();
        cout << "fin de cliente" << endl;
    }
}

void funcion_hebra_barbero(MRef<Barberia> monitor, int silla){
    while(true){
        monitor->siguienteCliente();
        CortarPeloAlCliente();
        cout << "Pelado cliente en silla " << silla << endl;
        monitor->finCliente(silla);
    }
}
void funcion_hebra_cliente(MRef<Barberia> monitor, int cliente){
    while(true){
        monitor->cortarPelo(cliente);
        EsperarFueraBarberia(cliente);
    }
}

int main()
{
    //Fumadores monitor;
    MRef<Barberia> monitor = Create<Barberia>();
    future<void> cliente[N];
    future<void> barbero1, barbero2;
    int i;

    barbero1 = async(launch::async, funcion_hebra_barbero, monitor, 1);
    barbero2 = async(launch::async, funcion_hebra_barbero, monitor, 2);
    
    for (i = 0; i < N; ++i){
        cliente[i] = async(launch::async, funcion_hebra_cliente, monitor, i);
    }

    for (i = 0; i < N; ++i){
        cliente[i].get();
    }
    barbero1.get(); barbero2.get();
}