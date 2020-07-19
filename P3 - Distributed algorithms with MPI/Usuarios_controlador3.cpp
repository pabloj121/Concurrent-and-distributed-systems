#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_usuarios = 5,
   num_cajas = 2,
   num_procesos  = num_usuarios + num_cajas + 1 ,
   etiq_solicitar = 1,
   etiq_liberar = 0,
   etiq_recibirCaja = 3,
   etiq_irAcaja = 5,
   etiq_irseCaja = 4,
   id_controlador = num_usuarios,
   id_caja1 = id_controlador+1,
   id_caja2 = id_caja1+1;
     
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

// ---------------------------------------------------------------------

void funcion_usuarios( int id )
{
	int valor;
  int cajaAsignada;
  MPI_Status estado ;
  
  while ( true ){
    //peticion de solicitar
	 cout <<"Usuario " <<id << "solicita caja " <<endl;
	 MPI_Ssend(&valor, 1, MPI_INT, id_controlador, etiq_solicitar, MPI_COMM_WORLD);
	 
	 // ... recibir caja    
    MPI_Recv(&cajaAsignada, 1, MPI_INT, id_controlador  ,  etiq_recibirCaja, MPI_COMM_WORLD,&estado);
    cout <<"Usuario " <<id << " recibe la caja." <<endl;
    
	 // ... Enviar caja
    cout <<"Usuario va hacia la caja" <<endl;
    MPI_Ssend(&valor, 1, MPI_INT, cajaAsignada, etiq_irAcaja , MPI_COMM_WORLD);
   
   //...Recibir caja
    cout <<"Usuario " << id << "termina en la caja " <<endl;
    MPI_Recv(&valor, 1, MPI_INT, cajaAsignada, etiq_irseCaja, MPI_COMM_WORLD,&estado);
   
   //Liberar
    cout <<"Usuario " <<id << "libera la caja" <<endl;
    MPI_Ssend (&valor, 1, MPI_INT, id_controlador, etiq_liberar, MPI_COMM_WORLD);
    
	//comienza a Esperar
    cout <<"Usuario " <<id <<" comienza a esperar" <<endl ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
   
 }
}
// ---------------------------------------------------------------------

void funcion_caja( int id )
{
  int valor, id_usuario ;  // valor recibido, identificador del usuario
  MPI_Status estado ;       // metadatos de las dos recepciones

  while ( true ){  
   // ...... recibir petición de cualquier usuario 
   
   MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_irAcaja , MPI_COMM_WORLD, &estado);
   
   // ...... guardar en 'id_usuario' el id. del emisor
  
  	id_usuario = estado.MPI_SOURCE;
    
   // ...... enviar liberación de usuario 'id_usuario' 
    
		MPI_Ssend(&valor, 1, MPI_INT, id_usuario, etiq_irseCaja , MPI_COMM_WORLD);
     
     cout <<"caja "<< id<< " ha sido liberado por usuario " <<id_usuario <<endl ;
  }
}
// ---------------------------------------------------------------------
void funcion_controlador(){
	MPI_Status estado;
	int tag_controlador;
	int cajas = 0;
	int valor;
  int cajaBuscadaParaElusuario;
	
	while ( true ){
		if(cajas < num_cajas){
			tag_controlador = MPI_ANY_TAG;
		}else{
			tag_controlador =  etiq_liberar;
    }
		
		MPI_Recv(&valor, 1, MPI_INT, MPI_ANY_SOURCE, tag_controlador, MPI_COMM_WORLD, &estado);
			
		if (estado.MPI_TAG == etiq_solicitar){

      if ( cajas == 0){
        cajaBuscadaParaElusuario = id_caja1;
      }else{
        cajaBuscadaParaElusuario = id_caja2;
      }

      cajas ++;
      cout<< "Se le asignara la caja  "<< cajaBuscadaParaElusuario << " al usuario "<< estado.MPI_SOURCE << endl;
      MPI_Ssend(&cajaBuscadaParaElusuario, 1, MPI_INT, estado.MPI_SOURCE ,etiq_recibirCaja , MPI_COMM_WORLD);
		}
		else if(estado.MPI_TAG == etiq_liberar){
			cajas--;
			cout<< "El usuario "<< estado.MPI_SOURCE << " ha soltado cajas. " << endl;
		}
	}
}

int main( int argc, char** argv ){
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

   if ( num_procesos == num_procesos_actual ){
      if (id_propio == id_controlador){
        funcion_controlador();
      }else if ( id_propio >= 0 && id_propio < num_usuarios ){
        funcion_usuarios( id_propio );
      }else{
        funcion_caja( id_propio );
      }

   }
   else{
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}