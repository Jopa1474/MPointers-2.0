#include "../mpointers/MPointer.h"
#include <iostream>

int main() {
    std::cout << "== TEST MPointer<int> CON ASIGNACIÓN ==\n" << std::endl;

    MPointer<int>::Init("localhost", 50051);

    MPointer<int> ptr1 = MPointer<int>::New();
    *ptr1 = 42;
    std::cout << "[ptr1] Valor asignado: 42 (ID: " << &ptr1 << ")" << std::endl;

    MPointer<int> ptr2;
    ptr2 = ptr1; // debería incrementar refcount
    std::cout << "[ptr2] Asignado desde ptr1 (ID: " << &ptr2 << ")" << std::endl;

    int val = *ptr2;
    std::cout << "[ptr2] Valor leído: " << val << std::endl;

    std::cout << "== FIN DEL TEST ==\n" << std::endl;
    return 0;
}
