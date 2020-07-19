#include <iostream>
#include <cassert>
#include <thread>
#include <stack>
#include <mutex>
#include <random>
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

//**********************************************************************
// variables compartidas

const int num_items = 60 ,   // número de items (50-100)
    tam_vec   = 10 ;   // tamaño del buffer (10-20), estrictamente menos que num_items
// no hacer caso a las 2 sig. variables
unsigned  cont_prod[num_items] = {0}, // contadores de verificación: producidos
          cont_cons[num_items] = {0}; // contadores de verificación: consumidos
int contador = 0;
// Declaración de cola y semáforos
Semaphore libres(tam_vec), ocupadas(0), m(1), libres2(tam_vec), ocupadas2(0);
stack<int> pila;

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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato()
{
   static int contador = 0 ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "producido: " << contador << endl << flush ;

   cont_prod[contador] ++ ;
   return contador++ ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
   contador++;
}

//----------------------------------------------------------------------

void test_contadores(){
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(int i){
  if (i==0){
    Semaphore l(libres), o(ocupadas);    
  }
  else if(i==1){
    Semaphore l(libres2), o(ocupadas2);  }
  else{
    cout << "valor erróneo introducido"<<endl;
  }

  for( unsigned i = 0 ; i < num_items ; i++ ){
    int dato = producir_dato() ;
    sem_wait(); 
    sem_wait(m);    
    pila.push(dato);
    sem_signal(m);
    cout << "Hemos insertado el valor " << dato << "en el buffer" << endl;
    sem_signal(o);   
  }
}

//----------------------------------------------------------------------
// sincronizacion con semaforos y sacr o meter elementos al vector con pila o cola ( en las 2 funciones)
// declarar los semaforos como variables globales
// diapositiva 11 practica 1

void funcion_hebra_consumidora(){
  for(unsigned i=0; i < num_items; i++){  
    sem_wait(ocupadas); 
    int dato;
    sem_wait(m);  
    dato = pila.top();
    pila.pop();
    sem_signal(m);
    cout << "Hemos extraido el valor " << dato << "del buffer" << endl;       
    sem_signal(libres);      
    consumir_dato(dato);
  }
}

void funcion_hebra_imprimir(){
  if(contador%5==0){
    cout << "Se ha consumido el elemento número " << contador << ", múltiplo de 5" << endl;
  }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

/*crear una hebra productora extra que se alternase 
una lectura de esa y otra de la que ya había, que se decidiese de 
manera aleatoria quién empezaba, que las dos hiciesen el mismo 
número de producciones y el último apartado decía crear una nueva
hebra imprimir, que en principio estuviese pausada, que cada vez 
que se consumiesen 5 elementos imprimiese que se había consumido 
tal elemento múltiplo de 5*/

//----------------------------------------------------------------------


//error: ‘::main’ must return ‘int’
int main(){
   cout << "--------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
        << "--------------------------------------------------------" << endl
        << flush ;

  // Inicializacion de hebras productoras aleatoria
  int inicializacion = aleatorio<0,1>();
  
  /*thread hebra_productora[2];
  thread hebra_consumidora(funcion_hebra_consumidora);
  thread hebra_productora[inicializacion](funcion_hebra_productora(inicializacion));

  // Hemos inicializado sólo una hebra, falta la otra
  if(inicializacion==0){
    hebra_productora[1]={funcion_hebra_productora(1)};
  }
  else{
    hebra_productora[0]=thread hebra_productora[0]; 
  }
  thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

  hebra_productora.join() ;
  hebra_consumidora.join() ;
  */

  future <void> productoras[2];
  future <void> imprimir(funcion_imprimir);
  productoras[inicializacion] = async(launch::async, funcion_hebra_productora,inicializacion);
  
  if(inicializacion==0){
    productoras[1] = async(launch::async, funcion_hebra_productora,1);
  }
  else{
    productoras[0] = async(launch::async, funcion_hebra_productora,0);
  }
  
  for(int i=0; i < n; i++){
    productoras[i].get();
  }

  imprimir.get();

  test_contadores();
//   return 0;
}
