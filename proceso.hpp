#ifndef PROCESO_HPP
#define PROCESO_HPP

#include <iostream>

using namespace std;

static int idUltimoProceso = 0;

class Proceso {
private:
    int idProceso;
    int tiempoEjecucion;
    int tiempoRestante;
    int tamanioProceso;
    int tamanioBloque; // Usado para potencias de 2 en Buddy System

public:
    Proceso() {
        this->idProceso = ++idUltimoProceso;
        this->tiempoEjecucion = 0;
        this->tiempoRestante = 0;
        this->tamanioProceso = 0;
        this->tamanioBloque = 0;
    }

    // Constructor para representar espacios libres en memoria (Id = 0)
    Proceso(int tamanioMemoria) {
        this->idProceso = 0;
        this->tamanioProceso = tamanioMemoria;
        this->tamanioBloque = tamanioMemoria;
        this->tiempoEjecucion = 0;
        this->tiempoRestante = 0;
    }

    int getId() const { return this->idProceso; }
    int getTiempoEjecucion() const { return this->tiempoEjecucion; }
    int getTiempoRestante() const { return this->tiempoRestante; }
    int getTamanioProceso() const { return this->tamanioProceso; }
    int getTamanioBloque() const { return this->tamanioBloque; }

    void setTiempoEjecucion(int tiempo) {
        this->tiempoEjecucion = tiempo;
        this->tiempoRestante = tiempo;
    }
    void setTiempoRestante(int tiempo) { this->tiempoRestante = tiempo; }
    void setTamanioProceso(int tamanio) { this->tamanioProceso = tamanio; }
    void setTamanioBloque(int tamanio) { this->tamanioBloque = tamanio; }

    void asignarDatos(const Proceso& p) {
        this->idProceso = p.idProceso;
        this->tiempoEjecucion = p.tiempoEjecucion;
        this->tiempoRestante = p.tiempoRestante;
        this->tamanioProceso = p.tamanioProceso;
        this->tamanioBloque = p.tamanioBloque;
    }

    friend ostream& operator<<(ostream& os, const Proceso& p) {
        os << "[" << p.getId() << "," << p.getTamanioProceso() << "," << p.getTiempoRestante() << "]";
        return os;
    }
};

#endif