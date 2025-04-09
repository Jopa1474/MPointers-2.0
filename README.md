# MPointers-2.0
Proyecto#1 de Algoritmos y Estructuras de Datos II


Proyecto en C++ que implementa un sistema de administración de memoria personalizado, inspirado en punteros inteligentes, comunicación remota y manejo automático de recursos.
Incluye recolección de basura, generación de informes de estado, y pruebas con estructuras dinámicas.

¿Qué hace este proyecto?

- Crea y gestiona bloques de memoria remota de distintos tipos (int, float, string, etc).

- Interactúa con ellos usando una clase MPointer<T>, similar a un puntero inteligente.

- Soporta operaciones básicas: asignación, lectura, copias y destrucción automática.

- Se conecta a un servidor de memoria usando gRPC (todo sucede remotamente).

- Libera automáticamente los bloques que ya no se están usando.

- Genera informes (dumps) mostrando el estado completo de la memoria.



Estructura del proyecto:

- memory_manager/: El servidor de memoria

- mpointers/: La clase MPointer<T>

- client/: Pruebas y clientes que usan MPointers

- dumps/: Aquí se guardan los informes automáticos de memoria


Pruebas realizadas:

- Crear y escribir valores remotos con MPointer<int>, MPointer<float>, etc.

- Asignación entre MPointers (incluye gestión de referencias).

- Liberación automática de memoria sin leaks.

- Implementación de una lista enlazada usando solamente MPointer.

- Recolección de basura (garbage collector) funcionando en segundo plano.

- Fusión de huecos de memoria (coalescing) para evitar fragmentación.

Ejemplo de uso con MPointer:




Compilar:

cd build
cmake ..
make


Ejecutar el servidor:


./build/memory_manager/memory_manager --port 50051 --memsize 10 --dumpFolder ./dumps
Ejecutar una prueba (ejemplo lista enlazada):


Ejecutar cliente con ejemplo de lista enlazada (Terminal diferente):

./build/client/test_linked_list



Tecnologías usadas:

- C++

- gRPC

- Protocol Buffers

- CMake

- Linux