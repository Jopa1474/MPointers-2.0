#include "../mpointers/MPointer.h"
#include <iostream>

int main() {
    std::cout << "== TEST Lista Enlazada Simulada con MPointer<int> y MPointer<uint32_t> ==\n" << std::endl;
    MPointer<int>::Init("localhost", 50051);

    // Cada nodo es representado por dos MPointers:
    // uno para el valor (int) y uno para el ID del siguiente nodo
    struct NodoSimulado {
        MPointer<int> valor;
        MPointer<uint32_t> siguiente_id;
    };

    // Creamos tres nodos
    NodoSimulado nodo1, nodo2, nodo3;
    nodo1.valor = MPointer<int>::New();
    nodo1.siguiente_id = MPointer<uint32_t>::New();

    nodo2.valor = MPointer<int>::New();
    nodo2.siguiente_id = MPointer<uint32_t>::New();

    nodo3.valor = MPointer<int>::New();
    nodo3.siguiente_id = MPointer<uint32_t>::New();

    // Asignar valores
    *nodo1.valor = 10;
    *nodo2.valor = 20;
    *nodo3.valor = 30;

    // Enlazar nodos (guardamos IDs)
    *nodo1.siguiente_id = &nodo2.valor;  // usamos & porque devuelve el ID
    *nodo2.siguiente_id = &nodo3.valor;
    *nodo3.siguiente_id = 0;  // null

    // Recorrer nodos (desde nodo1)
    MPointer<int> actual_valor = nodo1.valor;
    MPointer<uint32_t> actual_siguiente = nodo1.siguiente_id;

    while (&actual_valor != 0) {
        std::cout << "Valor: " << *actual_valor << std::endl;

        uint32_t next_id = *actual_siguiente;
        if (next_id == 0) break;

        // Creamos nuevos MPointer a partir del ID
        actual_valor = MPointer<int>(next_id);
        actual_siguiente = MPointer<uint32_t>(next_id + 1); // siguiente_id fue creado después del valor
    }

    std::cout << "== FIN DEL TEST ==\n" << std::endl;
    return 0;
}
