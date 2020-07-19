#include <mpi.h>
#include <thread>
#include <random>
#include <chrono>
#include <iostream>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

// mpicxx -std=c++11 -c filosofos-1camarero.cpp
// mpicxx -std=c++11 -o filosofos filosofos-1camarero.o
// mpirun -np 10 ./filosofos

const int
   num_filosofos = 5,
   num_procesos_efectivos = 2 * num_filosofos,
   num_procesos_esperados  = num_procesos_efectivos + 1,  
   id_camarero = num_procesos_efectivos;

const int
   etiq_sentarse = 0,
   etiq_levantarse = 1;

template< int min, int max > int aleatorio(){
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max );
  return distribucion_uniforme( generador );
}

void retraso_aleatorio( string mensaje, int id ){
  cout << "--- Filósofo " << id << " " << mensaje << " ---" << endl;
  sleep_for( milliseconds( aleatorio<10,100>() ) );
}

void funcion_filosofos( int id ){
  int id_ten_izq = (id + 1) % num_procesos_efectivos,
      id_ten_der = (id + num_procesos_efectivos - 1) % num_procesos_efectivos,
      peticion;

  while ( true ){
    cout << "Filósofo " << id << " solicita permiso para sentarse a la mesa" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD );

    cout << "Filósofo " << id << " solicita tenedor izquierdo (" << id_ten_izq << ")" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD );

    cout << "Filósofo " << id << " solicita tenedor derecho (" << id_ten_der << ")" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD );

    retraso_aleatorio("comienza a comer", id);

    cout << "Filósofo " << id << " suelta tenedor izquierdo (" << id_ten_izq << ")" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_izq, 0, MPI_COMM_WORLD );

    cout << "Filósofo " << id <<" suelta tenedor derecho (" << id_ten_der << ")" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_ten_der, 0, MPI_COMM_WORLD );

    cout << "Filósofo " << id << " solicita permiso para levantarse" << endl;
    MPI_Ssend( &peticion, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD );

    retraso_aleatorio("comienza a pensar", id);
  }
}

// ---------------------------------------------------------------------

void funcion_camarero(){
  int        filosofos_silla = 0, 
             peticion,               
             id_filosofo,            
             tag_bueno;      
  MPI_Status estado;

  while( true ){
     if ( filosofos_silla < num_filosofos - 1 ) 
        tag_bueno = MPI_ANY_TAG;       
     else                              
        tag_bueno = etiq_levantarse;   

     MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, tag_bueno, MPI_COMM_WORLD, &estado );
     id_filosofo = estado.MPI_SOURCE;

     switch( estado.MPI_TAG ){ 
        case etiq_levantarse:
           cout << "\tFilósofo " << id_filosofo << " se levanta de la mesa" << endl;
           filosofos_silla--;
           break;

        case etiq_sentarse:
           cout << "\tFilósofo " << id_filosofo << " se sienta a la mesa" << endl;
           filosofos_silla++;
           break;
     }
     cout << "\t --- Actualmente hay " << filosofos_silla << " filósofos sentados --- " << endl;
  }
}

// ---------------------------------------------------------------------

void funcion_tenedores( int id ){
  int        valor,
             id_filosofo ;
  MPI_Status estado ;

  while ( true ){
     // Recibir petición de cualquier filósofo
     MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &estado );
     id_filosofo = estado.MPI_SOURCE;
     cout << "\t\tTenedor " << id <<" cogido por filósofo " << id_filosofo << endl;

     // Recibir liberación de filósofo 'id_filosofo'
     MPI_Recv( &valor, 1, MPI_INT, id_filosofo, 0, MPI_COMM_WORLD, &estado );
     cout << "\t\tTenedor " << id << " liberado por filósofo " << id_filosofo << endl;
  }
}

//**********************************************************************
// Main
//----------------------------------------------------------------------

int main( int argc, char** argv ){
   int id_propio, num_procesos_actual;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos_esperados == num_procesos_actual ){
      if ( id_propio == id_camarero )
         funcion_camarero();
      else if ( id_propio % 2 == 0 )     
         funcion_filosofos( id_propio );
      else                              
         funcion_tenedores( id_propio );
   }
   else{
    
      if ( id_propio == 0 )
      {
        cout << "error: el número de procesos esperados es " << num_procesos_esperados
             << ", pero el número de procesos en ejecución es: " << num_procesos_actual << endl;
      }
   }

   MPI_Finalize();
   return 0;
}