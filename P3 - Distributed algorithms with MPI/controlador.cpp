#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

// mpicxx -std=c++11 -c ejercicioCAJAS.cpp
// mpicxx -std=c++11 -o cajas ejercicioCAJAS.o
// mpirun -np 10 ./cajas

//**********************************************************************
// Variables globales
//----------------------------------------------------------------------

// Parámetros del programa

struct Caja{
  int numero;
  bool libre;
};

const int
    id_controlador=5,
    num_procesos_esperado=8,
    tag_liberar=1,
    tag_solicitar=0;
   // tag_usuario=8,
    //tag_caja=9;

Caja uno;
  uno.libre = false;

Caja *cajas = new Caja[2];
//Caja cajas[2];
cajas[0].numero=6;
cajas[0].libre=true;
cajas[1].numero=7;
cajas[1].libre=true;

template< int min, int max > int aleatorio(){
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

int numeroCajaLibre(){
  int numero;
  for(int i=0;i<2;i++){
    if (cajas[i].libre==true){
      numero==cajas[i].numero;
    }
  }
  return numero;
}
void ocuparCaja(int num){
  for(int i=0;i<2;i++){
    if (cajas[i].numero==num){
      cajas[i].libre=false;
    }
  }
}
void liberarCaja(int num){
  for(int i=0;i<2;i++){
    if (cajas[i].numero==num){
      cajas[i].libre=true;
    }
  }
}



void funcion_controlador(){
  int tag,
      cajas_libres=2,
      num_caja;

    MPI_Status estado;
    
    if(cajas_libres>0){
      tag=MPI_ANY_TAG;
    }
    else{
      tag=tag_liberar;
    }

  MPI_Recv(&num_caja,1,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD,&estado);

  if(tag==tag_solicitar){
    cajas_libres--;
    num_caja=numeroCajaLibre();
    ocuparCaja(num_caja);
  }
  if (tag==tag_liberar){
    cajas_libres++;
    liberarCaja(num_caja);
  }
}

void funcion_usuario(){
  int valor,
      numero_caja;
  MPI_Status estado;

  MPI_Send(&valor,0,MPI_INT,id_controlador,tag_solicitar,MPI_COMM_WORLD);
  MPI_Recv(&numero_caja,0,MPI_INT,id_controlador,1,MPI_COMM_WORLD,&estado);

  // numero_caja=MPI_SOURCE.estado;

  MPI_Send(&valor,0,MPI_INT,numero_caja,1,MPI_COMM_WORLD);
  MPI_Recv(&valor,0,MPI_INT,numero_caja,1,MPI_COMM_WORLD,&estado);
  MPI_Send(&numero_caja,0,MPI_INT,id_controlador,tag_liberar,MPI_COMM_WORLD);

  sleep_for( milliseconds( aleatorio<110,200>()) );
}

void funcion_caja(){
  int valor,
      vi; 
  MPI_Status estado;

  MPI_Recv(&valor,0,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&estado);

  vi=estado.MPI_SOURCE;

  sleep_for( milliseconds( aleatorio<110,200>()) );
  MPI_Send(&valor,0,MPI_INT,vi,1,MPI_COMM_WORLD);
}

int main( int argc, char *argv[] ){
  
   int id_propio, num_procesos_actual;

   // inicializar MPI, leer identif. de proceso y número de procesos
   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperado == num_procesos_actual ){

      // ejecutar la operación apropiada a 'id_propio'
      if ( id_propio < id_controlador )
         funcion_usuario();
      else if ( id_propio == id_controlador )
         funcion_controlador();
      else{
         funcion_caja();
      }
   }
   else{
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   // al terminar el proceso, finalizar MPI
   MPI_Finalize( );
   return 0;
}
