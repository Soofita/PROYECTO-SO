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
    int tamanioBloqueBuddy; // Tamaño de potencia de 2 asignado en Buddy System

public:
    Proceso() {
        this->idProceso = ++idUltimoProceso;
        this->tiempoEjecucion = 0;
        this->tiempoRestante = 0;
        this->tamanioProceso = 0;
        this->tamanioBloqueBuddy = 0;
    }

    // Constructor para huecos / bloques libres
    Proceso(int tamanioMemoria) {
        this->idProceso = 0;
        this->tamanioProceso = tamanioMemoria;
        this->tamanioBloqueBuddy = tamanioMemoria;
        this->tiempoEjecucion = 0;
        this->tiempoRestante = 0;
    }

    int getId() const { return this->idProceso; }
    int getTiempoEjecucion() const { return this->tiempoEjecucion; }
    int getTiempoRestante() const { return this->tiempoRestante; }
    int getTamanioProceso() const { return this->tamanioProceso; }
    int getTamanioBloqueBuddy() const { return this->tamanioBloqueBuddy; }

    void setTiempoEjecucion(int tiempo) {
        this->tiempoEjecucion = tiempo;
        this->tiempoRestante = tiempo;
    }
    void setTiempoRestante(int tiempo) { this->tiempoRestante = tiempo; }
    void setTamanioProceso(int tamanio) { this->tamanioProceso = tamanio; }
    void setTamanioBloqueBuddy(int tamanio) { this->tamanioBloqueBuddy = tamanio; }

    void reemplazarProceso(const Proceso& p) {
        this->idProceso = p.idProceso;
        this->tiempoEjecucion = p.tiempoEjecucion;
        this->tiempoRestante = p.tiempoRestante;
        this->tamanioProceso = p.tamanioProceso;
        this->tamanioBloqueBuddy = p.tamanioBloqueBuddy;
    }

    friend ostream& operator<<(ostream& os, const Proceso& p) {
        os << "[" << p.getId() << "," << p.getTamanioProceso() << "," << p.getTiempoRestante() << "]";
        return os;
    }
};

#endif