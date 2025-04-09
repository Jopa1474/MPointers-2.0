// Este archivo implementa una prueba de lista enlazada utilizando exclusivamente objetos MPointer<T>.
// Cada "nodo" de la lista está simulado mediante dos bloques remotos: uno para el valor (int) y otro
// para el ID del siguiente nodo (uint32_t). No se usan punteros reales ni memoria local dinámica.
//
// El objetivo es demostrar que se puede construir una estructura enlazada básica usando solo IDs
// de bloques administrados por el Memory Manager remoto.

#include "../mpointers/MPointer.h"
#include <iostream>

int main() {
    std::cout << "== TEST Lista Enlazada Simulada con MPointer<int> y MPointer<uint32_t> ==\n" << std::endl;

    // Se establece la conexión con el servidor de memoria
    MPointer<int>::Init("localhost", 50051);

    // Simulación de un nodo compuesto por dos MPointer:
    // - uno para el valor entero
    // - otro para el id del siguiente nodo
    struct NodoSimulado {
        MPointer<int> valor;
        MPointer<uint32_t> siguiente_id;
    };

    // Crear 3 nodos enlazados: nodo1 -> nodo2 -> nodo3
    NodoSimulado nodo1, nodo2, nodo3;

    // Reservar memoria remota para cada componente
    nodo1.valor = MPointer<int>::New();
    nodo1.siguiente_id = MPointer<uint32_t>::New();

    nodo2.valor = MPointer<int>::New();
    nodo2.siguiente_id = MPointer<uint32_t>::New();

    nodo3.valor = MPointer<int>::New();
    nodo3.siguiente_id = MPointer<uint32_t>::New();

    // Asignar valores a cada nodo
    *nodo1.valor = 10;
    *nodo2.valor = 20;
    *nodo3.valor = 30;

    // Enlazar nodos mediante IDs remotos
    *nodo1.siguiente_id = &nodo2.valor;      // ID del valor de nodo2
    *nodo2.siguiente_id = &nodo3.valor;      // ID del valor de nodo3
    *nodo3.siguiente_id = 0;                 // null / fin de lista

    // Recorrer la lista desde nodo1 usando los IDs
    MPointer<int> actual_valor = nodo1.valor;
    MPointer<uint32_t> actual_siguiente = nodo1.siguiente_id;

    while (&actual_valor != 0) {
        std::cout << "Valor: " << *actual_valor << std::endl;

        uint32_t next_id = *actual_siguiente;
        if (next_id == 0) break;

        // Creamos los siguientes nodos utilizando los IDs almacenados
        actual_valor = MPointer<int>(next_id);
        actual_siguiente = MPointer<uint32_t>(next_id + 1); // El ID del campo siguiente_id se creó inmediatamente después
    }

    std::cout << "== FIN DEL TEST ==\n" << std::endl;
    return 0;
}
