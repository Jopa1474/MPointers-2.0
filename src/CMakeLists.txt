set(SOURCES main.cpp)

add_executable(MPointers ${SOURCES})

# Enlazar con bibliotecas gRPC
target_link_libraries(MPointers grpc++ protobuf)

