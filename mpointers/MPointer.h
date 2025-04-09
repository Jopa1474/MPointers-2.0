// Esta clase template actúa como un puntero inteligente que permite manipular valores
// remotos almacenados en el servidor MemoryManager. Toda la memoria gestionada está
// del lado del servidor, y MPointer se encarga de realizar las llamadas gRPC necesarias
// para crear, leer, escribir y gestionar el ciclo de vida de los bloques de memoria remota.
//
// Principales características:
// - Comportamiento similar a un puntero: *, &, asignación.
// - Gestión automática del refCount remoto (garbage collection).
// - Utiliza una clase proxy (RemoteReference) para lectura/escritura.


#ifndef MPOINTER_H
#define MPOINTER_H

#include <memory>
#include <string>
#include <iostream>
#include <type_traits>
#include <grpcpp/grpcpp.h>
#include "../generated/memory.grpc.pb.h"
#include "MPointerManager.h"

template <typename T>
class MPointer {
public:

    // Clase anidada utilizada como proxy de lectura/escritura al hacer *ptr
    class RemoteReference {
    public:
        explicit RemoteReference(uint32_t _id) : id(_id) {}

        // Permite leer: int x = *ptr;
        operator T() const {
            std::string value = MPointerManager::getInstance().get(id);
            return MPointer<T>::parse(value);
        }

        // Permite escribir: *ptr = 99;
        RemoteReference& operator=(const T& value) {
            MPointerManager::getInstance().set(id, MPointer<T>::to_string_value(value));
            return *this;
        }

    private:
        uint32_t id;
    };

    // Constructor por defecto (puntero nulo)
    MPointer() : id(0) {}

    // Constructor explícito con ID remoto
    explicit MPointer(uint32_t _id) : id(_id) {}

    // Destructor: notifica al MemoryManager que se destruyó la referencia
    ~MPointer() {
        if (id != 0) {
            MPointerManager::getInstance().decreaseRef(id);
        }
    }

    // Constructor de copia: aumenta el refCount en el servidor
    MPointer(const MPointer& other) : id(other.id) {
        if (id != 0) {
            MPointerManager::getInstance().increaseRef(id);
        }
    }

    // Operador de asignación entre MPointers
    MPointer& operator=(const MPointer& other) {
        if (id == other.id) return *this;

        // Decrementa ref anterior
        if (id != 0) {
            MPointerManager::getInstance().decreaseRef(id);
        }

        // Asigna nuevo ID
        id = other.id;

        // Incrementa nueva referencia
        if (id != 0) {
            MPointerManager::getInstance().increaseRef(id);
        }

        return *this;
    }

    // Permite usar *ptr para acceder a RemoteReference
    RemoteReference operator*() {
        return RemoteReference(id);
    }

    // Simula operador &: retorna el ID del bloque (no dirección real)
    uint32_t operator&() const {
        return id;
    }

    // Establece la conexión con el servidor (se debe llamar antes de usar MPointer)
    static void Init(const std::string& host, int port) {
        MPointerManager::init(host, port);
    }

    // Solicita la creación de un nuevo bloque remoto para este tipo
    static MPointer<T> New() {
        uint32_t newId = MPointerManager::getInstance().create(sizeof(T), getTypeName());
        return MPointer<T>(newId);
    }

private:
    uint32_t id;

    // Determina el nombre del tipo actual para enviar al servidor
    static std::string getTypeName() {
        if (std::is_same<T, int>::value) return "int";
        if (std::is_same<T, float>::value) return "float";
        if (std::is_same<T, double>::value) return "double";
        if (std::is_same<T, std::string>::value) return "string";
        if (std::is_same<T, uint32_t>::value) return "uint32_t";
        throw std::runtime_error("Tipo no soportado en MPointer<T>");
    }

    // Convierte string recibido desde el servidor al tipo T
    static T parse(const std::string& value) {
        if constexpr (std::is_same<T, int>::value) return std::stoi(value);
        if constexpr (std::is_same<T, uint32_t>::value) return static_cast<uint32_t>(std::stoul(value));
        if constexpr (std::is_same<T, float>::value) return std::stof(value);
        if constexpr (std::is_same<T, double>::value) return std::stod(value);
        if constexpr (std::is_same<T, std::string>::value) return value;
        throw std::runtime_error("Tipo no soportado en parse<T>");
    }

    // Convierte T a string para enviarlo al servidor
    static std::string to_string_value(const T& value) {
        if constexpr (std::is_same<T, int>::value) return std::to_string(value);
        if constexpr (std::is_same<T, uint32_t>::value) return std::to_string(static_cast<unsigned long>(value));
        if constexpr (std::is_same<T, float>::value) return std::to_string(value);
        if constexpr (std::is_same<T, double>::value) return std::to_string(value);
        if constexpr (std::is_same<T, std::string>::value) return value;
        throw std::runtime_error("Tipo no soportado en to_string_value<T>");
    }
};

#endif // MPOINTER_H
