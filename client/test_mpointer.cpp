// Este archivo prueba el funcionamiento básico de la clase MPointer<T>.
// Se valida que el sistema pueda:
// - Establecer una conexión gRPC con el servidor Memory Manager
// - Crear un bloque de memoria remota
// - Asignar un valor al bloque remotamente usando el operador *
// - Leer el valor desde la memoria remota usando el operador *
// - Liberar la referencia al salir del alcance (destructor)

#include "../mpointers/MPointer.h"
#include <iostream>

int main() {
    std::cout << "== TEST MPointer<int> ==\n" << std::endl;

    // Paso 1: Inicializar conexión gRPC al servidor remoto
    MPointer<int>::Init("localhost", 50051);

    // Paso 2: Crear un nuevo bloque remoto de tipo int
    MPointer<int> ptr = MPointer<int>::New();
    std::cout << "[MPointer] ID asignado: " << &ptr << std::endl;

    // Paso 3: Asignar un valor (escritura remota)
    *ptr = 99;
    std::cout << "[MPointer] Valor asignado: 99" << std::endl;

    // Paso 4: Leer el valor (lectura remota)
    int valor = *ptr;
    std::cout << "[MPointer] Valor leído: " << valor << std::endl;

    // Al finalizar: el destructor del MPointer notificará al servidor
    // para disminuir el refcount del bloque asociado
    std::cout << "== FIN DEL TEST ==\n" << std::endl;
    return 0;
}

