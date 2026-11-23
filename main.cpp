
#include <iostream>
#include <deque>
#include <list>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <iterator>
#include "proceso.hpp"

using namespace std;

// Constantes de memoria (en KB)
const int MEM_1MB = 1024;
const int MEM_4MB = 4096;
const int MEM_8MB = 8192;
const int MIN_SPLIT_BUDDY = 32;

// Algoritmos asignados al equipo
enum Algoritmo {
    PRIMER_AJUSTE = 1,
    BUDDY_SYSTEM = 2
};

// Parámetros principales del sistema
int quantumSistema = 0;
int tamanoMaxProceso = 0;
int quantumMaxProceso = 0;
int tamanoMemoriaTotal = MEM_4MB;
Algoritmo algoritmoActual;

// Prototipos de funciones del avance
void MostrarMemoria(const list<Proceso>& memoria);
void MostrarColaRR(const deque<Proceso>& colaRR);
bool AsignarPrimerAjuste(list<Proceso>& memoria, Proceso& p);
bool AsignarBuddySystem(list<Proceso>& memoria, Proceso& p);
int CalcularBloqueBuddy(int tamanoProceso);
void EjecutarRoundRobin(deque<Proceso>& colaRR, list<Proceso>& memoria);
void LiberarMemoria(list<Proceso>& memoria, int idProceso);

int main() {
    srand(time(NULL));

    list<Proceso> memoria;
    deque<Proceso> colaRR;

    cout << "=== AVANCE 1: GESTION DE MEMORIA Y ROUND ROBIN ===" << endl;
    cout << "Algoritmos: Primer Ajuste / Buddy System + Round Robin" << endl << endl;

    // 1. Configuración de memoria y cuantos del sistema
    cout << "Seleccione tamanio de memoria (1: 1MB, 2: 4MB, 3: 8MB): ";
    int opMem;
    cin >> opMem;
    if (opMem == 1) tamanoMemoriaTotal = MEM_1MB;
    else if (opMem == 3) tamanoMemoriaTotal = MEM_8MB;
    else tamanoMemoriaTotal = MEM_4MB;

    cout << "Ingrese el quantum del sistema: ";
    cin >> quantumSistema;

    cout << "Ingrese el tamanio maximo por proceso (KB): ";
    cin >> tamanoMaxProceso;

    cout << "Ingrese el quantum maximo por proceso: ";
    cin >> quantumMaxProceso;

    cout << "Seleccione algoritmo (1: Primer Ajuste, 2: Buddy System): ";
    int opAlg;
    cin >> opAlg;
    algoritmoActual = (opAlg == 2) ? BUDDY_SYSTEM : PRIMER_AJUSTE;

    // Inicializar toda la memoria como un bloque libre (Id = 0)
    memoria.push_back(Proceso(tamanoMemoriaTotal));

    cout << "\nMemoria inicial: ";
    MostrarMemoria(memoria);
    cout << endl << endl;

    // Prueba controlada de ciclos para demostrar generacion de IDs, cuantos y algoritmos
    int ciclosPrueba = 8;
    for (int ciclo = 1; ciclo <= ciclosPrueba; ciclo++) {
        cout << "--- Ciclo " << ciclo << " ---" << endl;

        // Generacion de proceso con ID autoincremental y cuanto aleatorio
        Proceso nuevo;
        nuevo.setTamanioProceso(rand() % tamanoMaxProceso + 1);
        nuevo.setTiempoEjecucion(rand() % quantumMaxProceso + 1);

        cout << "Proceso generado (ID=" << nuevo.getId()<< ", Tam=" << nuevo.getTamanioProceso()<< "KB, Quantum=" << nuevo.getTiempoRestante() << ")" << endl;

        bool asignado = false;
        if (algoritmoActual == PRIMER_AJUSTE) {
            asignado = AsignarPrimerAjuste(memoria, nuevo);
        } else {
            asignado = AsignarBuddySystem(memoria, nuevo);
        }

        if (asignado) {
            colaRR.push_back(nuevo);
        } else {
            cout << "-> Sin espacio disponible en memoria para el proceso " << nuevo.getId() << endl;
        }

        cout << "Memoria: ";
        MostrarMemoria(memoria);
        cout << endl;

        // Atencion de cuantos con Round Robin
        EjecutarRoundRobin(colaRR, memoria);

        cout << endl;
        Sleep(800);
    }

    cout << "Fin de la prueba de avance." << endl;
    return 0;
}
// ==================== ALGORITMOS DE ASIGNACIÓN ====================

bool AsignarPrimerAjuste(list<Proceso>& memoria, Proceso& p) {
    for (auto it = memoria.begin(); it != memoria.end(); ++it) {
        if (it->getId() == 0 && it->getTamanioProceso() >= p.getTamanioProceso()) {
            int sobrante = it->getTamanioProceso() - p.getTamanioProceso();
            if (sobrante > 0) {
                memoria.insert(next(it), Proceso(sobrante));
            }
            it->asignarDatos(p);
            return true;
        }
    }
    return false;
}

int CalcularBloqueBuddy(int tamanoProceso) {
    int bloque = MIN_SPLIT_BUDDY; // Mínimo 32 KB
    while (bloque < tamanoProceso) {
        bloque *= 2;
    }
    return bloque;
}

bool AsignarBuddySystem(list<Proceso>& memoria, Proceso& p) {
    int bloqueNecesario = CalcularBloqueBuddy(p.getTamanioProceso());

    for (auto it = memoria.begin(); it != memoria.end(); ++it) {
        if (it->getId() == 0 && it->getTamanioBloque() >= bloqueNecesario) {
            // Dividir el bloque en mitades hasta llegar al tamaño potencia de 2 requerido
            while (it->getTamanioBloque() > bloqueNecesario &&(it->getTamanioBloque() / 2) >= MIN_SPLIT_BUDDY) {
                int mitad = it->getTamanioBloque() / 2;
                it->setTamanioProceso(mitad);
                it->setTamanioBloque(mitad);
                memoria.insert(next(it), Proceso(mitad));
            }

            p.setTamanioBloque(bloqueNecesario);
            it->asignarDatos(p);
            return true;
        }
    }
    return false;
}

// ==================== ROUND ROBIN Y LIBERACIÓN BÁSICA ====================

void EjecutarRoundRobin(deque<Proceso>& colaRR, list<Proceso>& memoria) {
    if (colaRR.empty()) {
        cout << "Cola RR vacia." << endl;
        return;
    }

    Proceso actual = colaRR.front();
    colaRR.pop_front();

    cout << "Atendiendo en CPU (Quantum Sistema=" << quantumSistema << "): " << actual << endl;

    if (actual.getTiempoRestante() > quantumSistema) {
        actual.setTiempoRestante(actual.getTiempoRestante() - quantumSistema);
        colaRR.push_back(actual);

        // Actualizar quantum restante en la vista de memoria
        for (auto& bloque : memoria) {
            if (bloque.getId() == actual.getId()) {
                bloque.setTiempoRestante(actual.getTiempoRestante());
                break;
            }
        }
    } else {
        actual.setTiempoRestante(0);
        cout << "-> Proceso " << actual.getId() << " termino su ejecucion." << endl;
        LiberarMemoria(memoria, actual.getId());
    }

    cout << "Cola RR: ";
    MostrarColaRR(colaRR);
    cout << endl;
}

void LiberarMemoria(list<Proceso>& memoria, int idProceso) {
    for (auto it = memoria.begin(); it != memoria.end(); ++it) {
        if (it->getId() == idProceso) {
            int tamLiberado = (algoritmoActual == BUDDY_SYSTEM) ? it->getTamanioBloque() : it->getTamanioProceso();
            it->asignarDatos(Proceso(tamLiberado));
            // TODO: Implementar condensacion de huecos adyacentes y fusion de buddies en la siguiente entrega
            return;
        }
    }
}

// ==================== VISUALIZACIÓN EN CONSOLA ====================

void MostrarMemoria(const list<Proceso>& memoria) {
    for (const auto& bloque : memoria) {
        if (algoritmoActual == BUDDY_SYSTEM) {
            cout << "[" << bloque.getId() << "," << bloque.getTamanioProceso()<< "/" << bloque.getTamanioBloque() << "K," << bloque.getTiempoRestante() << "]";
        } else {
            cout << bloque;
        }
    }
}

void MostrarColaRR(const deque<Proceso>& colaRR) {
    if (colaRR.empty()) {
        cout << "(Vacia)";
        return;
    }
    for (const auto& p : colaRR) {
        cout << p << " ";
    }
}
