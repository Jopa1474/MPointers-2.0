// Este archivo prueba el comportamiento del operador de asignación entre objetos MPointer<T>.
// El objetivo es verificar que al copiar un MPointer a otro, el nuevo objeto comparte el mismo
// ID remoto y que el reference count (refcount) se incrementa correctamente.
//
// Este comportamiento es fundamental para evitar duplicación innecesaria de bloques y asegurar
// una gestión de memoria remota eficiente.

#include "../mpointers/MPointer.h"
#include <iostream>

int main() {
    std::cout << "== TEST MPointer<int> CON ASIGNACIÓN ==\n" << std::endl;

    // Inicializa la conexión gRPC al servidor del Memory Manager
    MPointer<int>::Init("localhost", 50051);

    // Crea un nuevo bloque remoto y le asigna el valor 42
    MPointer<int> ptr1 = MPointer<int>::New();
    *ptr1 = 42;
    std::cout << "[ptr1] Valor asignado: 42 (ID: " << &ptr1 << ")" << std::endl;

    // Crea un segundo MPointer y lo iguala al primero
    // Esto debe copiar el ID y aumentar el refcount remotamente
    MPointer<int> ptr2;
    ptr2 = ptr1;
    std::cout << "[ptr2] Asignado desde ptr1 (ID: " << &ptr2 << ")" << std::endl;

    // Accede al valor usando ptr2 y lo imprime
    int val = *ptr2;
    std::cout << "[ptr2] Valor leído: " << val << std::endl;

    std::cout << "== FIN DEL TEST ==\n" << std::endl;
    return 0;
}
