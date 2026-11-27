#include <iostream>
#include <iomanip>
#include <deque>
#include <list>
#include <cstdlib>
#include <ctime>
#include <conio.h>
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
bool detenerSimulacion = false;

// Variables para controlar proceso en espera cuando la memoria está llena
bool hayProcesoEnEspera = false;
Proceso procesoEnEspera(0);

// Prototipos de funciones
void MostrarMemoria(const list<Proceso>& memoria);
void MostrarColaRR(const deque<Proceso>& colaRR);
bool AsignarPrimerAjuste(list<Proceso>& memoria, Proceso& p);
bool AsignarBuddySystem(list<Proceso>& memoria, Proceso& p);
int CalcularBloqueBuddy(int tamanoProceso);
void EjecutarRoundRobin(deque<Proceso>& colaRR, list<Proceso>& memoria);
void LiberarMemoria(list<Proceso>& memoria, int idProceso);
void PausaControlada(int milisegundos);

void ImprimirItem(int id, int tamanio, int q) {
    cout << "[" << id << "," << tamanio << "," << q << "]";
}

int main() {
    SetConsoleTitleA("Simulador");
    srand(time(NULL));

    list<Proceso> memoria;
    deque<Proceso> colaRR;

    cout << "+--------------------------------------------------------+" << endl;
    cout << "|     SIMULADOR DE MEMORIA Y PLANIFICACION ROUND ROBIN   |" << endl;
    cout << "|         Primer Ajuste / Buddy System + Round Robin     |" << endl;
    cout << "+--------------------------------------------------------+" << endl << endl;

    // 1. Configuración de memoria y cuantos del sistema
    cout << "  [1] Tamanio de memoria (1: 1MB, 2: 4MB, 3: 8MB): ";
    int opMem;
    cin >> opMem;
    if (opMem == 1) tamanoMemoriaTotal = MEM_1MB;
    else if (opMem == 3) tamanoMemoriaTotal = MEM_8MB;
    else tamanoMemoriaTotal = MEM_4MB;

    cout << "  [2] Quantum del sistema: ";
    cin >> quantumSistema;

    cout << "  [3] Tamanio maximo por proceso (KB): ";
    cin >> tamanoMaxProceso;

    cout << "  [4] Quantum maximo por proceso: ";
    cin >> quantumMaxProceso;

    cout << "  [5] Algoritmo (1: Primer Ajuste, 2: Buddy System): ";
    int opAlg;
    cin >> opAlg;
    algoritmoActual = (opAlg == 2) ? BUDDY_SYSTEM : PRIMER_AJUSTE;

    // Inicializar toda la memoria como un bloque libre (Id = 0)
    memoria.push_back(Proceso(tamanoMemoriaTotal));

    cout << "\n[ESPACIO: Pausar/Reanudar | ESC: Detener simulacion]" << endl;
    cout << "\nCompactando huecos" << endl;
    MostrarMemoria(memoria);
    cout << endl << endl;

    // Ciclos infinitos de simulación hasta presionar ESC
    int ciclo = 1;
    while (!detenerSimulacion) {
        cout << "--- Ciclo " << ciclo << " ---" << endl;

        // Si no había un proceso atorado esperando memoria, generamos uno nuevo
        if (!hayProcesoEnEspera) {
            Proceso nuevo;
            nuevo.setTamanioProceso(rand() % tamanoMaxProceso + 1);
            nuevo.setTiempoEjecucion(rand() % quantumMaxProceso + 1);
            procesoEnEspera.asignarDatos(nuevo);
        }

        bool asignado = false;
        if (algoritmoActual == PRIMER_AJUSTE) {
            asignado = AsignarPrimerAjuste(memoria, procesoEnEspera);
        } else {
            asignado = AsignarBuddySystem(memoria, procesoEnEspera);
        }

        if (asignado) {
            cout << "Cargando Proceso ";
            ImprimirItem(procesoEnEspera.getId(), procesoEnEspera.getTamanioProceso(), procesoEnEspera.getTiempoRestante());
            cout << endl;
            colaRR.push_back(procesoEnEspera);
            hayProcesoEnEspera = false;
            MostrarMemoria(memoria);
            cout << endl;
        } else {
            hayProcesoEnEspera = true;
            cout << "Esperando espacio en memoria para Proceso ";
            ImprimirItem(procesoEnEspera.getId(), procesoEnEspera.getTamanioProceso(), procesoEnEspera.getTiempoRestante());
            cout << endl;
        }

        // Atención de cuantos con Round Robin
        EjecutarRoundRobin(colaRR, memoria);

        cout << endl;
        ciclo++;
        PausaControlada(800);
    }

    cout << "Simulacion detenida por el usuario en el ciclo " << (ciclo - 1) << "." << endl;
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

    // Buscar de izquierda a derecha el bloque libre más pequeño que alcance (>= bloqueNecesario)
    // Así se aprovechan primero los huecos pequeños a la izquierda antes de partir bloques grandes
    auto mejorIt = memoria.end();
    int menorTamDisponible = tamanoMemoriaTotal + 1;

    for (auto it = memoria.begin(); it != memoria.end(); ++it) {
        if (it->getId() == 0 && it->getTamanioBloque() >= bloqueNecesario) {
            if (it->getTamanioBloque() < menorTamDisponible) {
                menorTamDisponible = it->getTamanioBloque();
                mejorIt = it;
                if (menorTamDisponible == bloqueNecesario) {
                    break;
                }
            }
        }
    }

    if (mejorIt == memoria.end()) {
        return false;
    }

    // Dividir de izquierda a derecha: el bloque seleccionado se va partiendo en mitades
    // y siempre se asigna el proceso en la mitad izquierda más pequeña
    while (mejorIt->getTamanioBloque() > bloqueNecesario && (mejorIt->getTamanioBloque() / 2) >= MIN_SPLIT_BUDDY) {
        int mitad = mejorIt->getTamanioBloque() / 2;
        mejorIt->setTamanioProceso(mitad);
        mejorIt->setTamanioBloque(mitad);
        memoria.insert(next(mejorIt), Proceso(mitad));
    }

    p.setTamanioBloque(bloqueNecesario);
    mejorIt->asignarDatos(p);
    return true;
}

// ==================== ROUND ROBIN Y LIBERACIÓN CONDENSACIÓN ====================

void EjecutarRoundRobin(deque<Proceso>& colaRR, list<Proceso>& memoria) {
    if (colaRR.empty()) {
        return;
    }

    Proceso actual = colaRR.front();
    colaRR.pop_front();

    cout << "Procesando ";
    ImprimirItem(actual.getId(), actual.getTamanioProceso(), actual.getTiempoRestante());
    cout << endl;

    MostrarColaRR(colaRR);
    cout << endl;

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
        MostrarMemoria(memoria);
        cout << endl;
    } else {
        actual.setTiempoRestante(0);
        cout << "Descargando proceso ";
        ImprimirItem(actual.getId(), actual.getTamanioProceso(), 0);
        cout << endl;
        LiberarMemoria(memoria, actual.getId());
    }
}

void LiberarMemoria(list<Proceso>& memoria, int idProceso) {
    for (auto it = memoria.begin(); it != memoria.end(); ++it) {
        if (it->getId() == idProceso) {
            int tamLiberado = (algoritmoActual == BUDDY_SYSTEM) ? it->getTamanioBloque() : it->getTamanioProceso();
            it->asignarDatos(Proceso(tamLiberado));

            // 1. Mostrar estado inmediato al liberar el bloque (antes de condensar)
            MostrarMemoria(memoria);
            cout << endl;

            // 2. Condensación / Fusión de huecos adyacentes
            if (algoritmoActual == PRIMER_AJUSTE) {
                bool huboCondensacion = false;

                // Revisar hueco siguiente
                auto itSig = next(it);
                if (itSig != memoria.end() && itSig->getId() == 0) {
                    it->setTamanioProceso(it->getTamanioProceso() + itSig->getTamanioProceso());
                    it->setTamanioBloque(it->getTamanioProceso());
                    memoria.erase(itSig);
                    huboCondensacion = true;
                }

                // Revisar hueco anterior
                if (it != memoria.begin()) {
                    auto itAnt = prev(it);
                    if (itAnt->getId() == 0) {
                        itAnt->setTamanioProceso(itAnt->getTamanioProceso() + it->getTamanioProceso());
                        itAnt->setTamanioBloque(itAnt->getTamanioProceso());
                        memoria.erase(it);
                        huboCondensacion = true;
                    }
                }

                if (huboCondensacion) {
                    cout << "Condensando huecos" << endl;
                    MostrarMemoria(memoria);
                    cout << endl;
                }
            } else {
                // Fusión en cascada de bloques compañeros (Buddy System)
                bool huboFusion = false;
                bool fusionando = true;

                while (fusionando) {
                    fusionando = false;
                    int offset = 0;
                    for (auto b = memoria.begin(); b != memoria.end(); ++b) {
                        int tamActual = b->getTamanioBloque();
                        auto bSig = next(b);

                        if (bSig != memoria.end() &&
                            b->getId() == 0 && bSig->getId() == 0 &&
                            tamActual == bSig->getTamanioBloque() &&
                            (offset % (2 * tamActual) == 0)) {

                            int nuevoTam = tamActual * 2;
                            b->setTamanioProceso(nuevoTam);
                            b->setTamanioBloque(nuevoTam);
                            memoria.erase(bSig);

                            huboFusion = true;
                            fusionando = true;
                            break;
                        }
                        offset += tamActual;
                    }
                }

                if (huboFusion) {
                    cout << "Fusionando buddies" << endl;
                    MostrarMemoria(memoria);
                    cout << endl;
                }
            }
            return;
        }
    }
}

// ==================== VISUALIZACIÓN ====================

void MostrarMemoria(const list<Proceso>& memoria) {
    int offset = 0;
    int mitadMemoria = tamanoMemoriaTotal / 2;

    for (const auto& bloque : memoria) {
        if (algoritmoActual == BUDDY_SYSTEM) {
            int tamBloque = bloque.getTamanioBloque();
            ImprimirItem(bloque.getId(), tamBloque, (bloque.getId() == 0 ? 0 : bloque.getTiempoRestante()));
            offset += tamBloque;

            // Un único '*' justo en la frontera donde se divide la mitad izquierda de la derecha
            if (offset == mitadMemoria) {
                cout << "*";
            }
        } else {
            if (bloque.getId() == 0) {
                ImprimirItem(0, bloque.getTamanioProceso(), 0);
            } else {
                ImprimirItem(bloque.getId(), bloque.getTamanioProceso(), bloque.getTiempoRestante());
            }
        }
    }
}

void MostrarColaRR(const deque<Proceso>& colaRR) {
    cout << "Cola RR";
    for (const auto& p : colaRR) {
        cout << " ";
        ImprimirItem(p.getId(), p.getTamanioProceso(), p.getTiempoRestante());
    }
}

void PausaControlada(int milisegundos) {
    bool pausado = false;
    int transcurrido = 0;
    const int paso = 50;

    while (transcurrido < milisegundos || pausado) {
        if (_kbhit()) {
            char tecla = _getch();
            if (tecla == 27) { // ESC
                detenerSimulacion = true;
                return;
            } else if (tecla == 32) { // ESPACIO
                pausado = !pausado;
                if (pausado) {
                    cout << "[PAUSA - Presione espacio para continuar o esc para salir]" << endl;
                } else {
                    cout << "[REANUDADO]" << endl;
                }
            }
        }
        Sleep(paso);
        if (!pausado) {
            transcurrido += paso;
        }
    }
}