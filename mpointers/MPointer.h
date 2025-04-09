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
    // Proxy que permite leer y escribir remotamente usando el operador *
    class RemoteReference {
    public:
        explicit RemoteReference(uint32_t _id) : id(_id) {}

        // Leer valor: permite int x = *ptr;
        operator T() const {
            std::string value = MPointerManager::getInstance().get(id);
            return MPointer<T>::parse(value);
        }

        // Escribir valor: permite *ptr = 99;
        RemoteReference& operator=(const T& value) {
            MPointerManager::getInstance().set(id, MPointer<T>::to_string_value(value));
            return *this;
        }

    private:
        uint32_t id;
    };

    // Constructor por defecto
    MPointer() : id(0) {}

    // Constructor con ID (interno)
    explicit MPointer(uint32_t _id) : id(_id) {}

    // Destructor → notifica al Memory Manager
    ~MPointer() {
        if (id != 0) {
            MPointerManager::getInstance().decreaseRef(id);
        }
    }

    // Copia → aumenta el refcount
    MPointer(const MPointer& other) : id(other.id) {
        if (id != 0) {
            MPointerManager::getInstance().increaseRef(id);
        }
    }

    // Asignación entre MPointers → copia ID y aumenta refcount
    MPointer& operator=(const MPointer& other) {
        if (id == other.id) return *this;

        if (id != 0) {
            MPointerManager::getInstance().decreaseRef(id);
        }

        id = other.id;

        if (id != 0) {
            MPointerManager::getInstance().increaseRef(id);
        }

        return *this;
    }

    // Operador * sobrecargado → devuelve proxy de lectura/escritura
    RemoteReference operator*() {
        return RemoteReference(id);
    }

    // Operador & → devuelve ID como simulación de dirección
    uint32_t operator&() const {
        return id;
    }

    // Método estático para inicializar conexión gRPC
    static void Init(const std::string& host, int port) {
        MPointerManager::init(host, port);
    }

    // Método estático para crear un nuevo MPointer remoto
    static MPointer<T> New() {
        uint32_t newId = MPointerManager::getInstance().create(sizeof(T), getTypeName());
        return MPointer<T>(newId);
    }

private:
    uint32_t id;

    static std::string getTypeName() {
        if (std::is_same<T, int>::value) return "int";
        if (std::is_same<T, float>::value) return "float";
        if (std::is_same<T, double>::value) return "double";
        if (std::is_same<T, std::string>::value) return "string";
        if (std::is_same<T, uint32_t>::value) return "uint32_t";
        throw std::runtime_error("Tipo no soportado en MPointer<T>");
    }

    static T parse(const std::string& value) {
        if constexpr (std::is_same<T, int>::value) return std::stoi(value);
        if constexpr (std::is_same<T, uint32_t>::value) return static_cast<uint32_t>(std::stoul(value));
        if constexpr (std::is_same<T, float>::value) return std::stof(value);
        if constexpr (std::is_same<T, double>::value) return std::stod(value);
        if constexpr (std::is_same<T, std::string>::value) return value;
        throw std::runtime_error("Tipo no soportado en parse<T>");
    }

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
