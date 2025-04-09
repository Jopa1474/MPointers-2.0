#include "../mpointers/MPointer.h"
#include <iostream>

int main() {
    std::cout << "== TEST MPointer<int> ==\n" << std::endl;

    // Paso 1: Inicializar conexión gRPC
    MPointer<int>::Init("localhost", 50051);

    // Paso 2: Crear un nuevo MPointer remoto
    MPointer<int> ptr = MPointer<int>::New();
    std::cout << "[MPointer] ID asignado: " << &ptr << std::endl;

    // Paso 3: Asignar un valor
    *ptr = 99;
    std::cout << "[MPointer] Valor asignado: 99" << std::endl;

    // Paso 4: Leer el valor
    int valor = *ptr;
    std::cout << "[MPointer] Valor leído: " << valor << std::endl;

    // Al finalizar: destructor libera referencia automáticamente
    std::cout << "== FIN DEL TEST ==\n" << std::endl;
    return 0;
}
