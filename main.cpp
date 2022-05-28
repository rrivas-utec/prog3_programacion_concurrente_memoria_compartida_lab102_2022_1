#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>
using namespace std;

mutex mtx;

class desbloqueo_automatico {
    mutex& mtx;
public:
    explicit desbloqueo_automatico(mutex& mtx): mtx(mtx) {
        mtx.lock();
    }
    ~desbloqueo_automatico() {
        mtx.unlock();
    }
};

void incrementar(int& x) {
    // Inicio de sección crítica
//    lock_guard lg(mtx);         // Este es fijo
    unique_lock ul(mtx, defer_lock);        // Es mas flexibilidad
//    mtx.lock();   // Al momento de construir
//    this_thread::sleep_for(chrono::milliseconds (10));
    ul.lock();
    x = x + 1;
//    ul.unlock();
    // Fin de sección crítica
//    mtx.unlock(); // Al momento de destruir
}

void ejemplo_1_race_condition_mutex() {
    // Cantidad de hilos
    int n_repeticion = 100;
    int n_hilos = 100;
    // Colección de hilos
    vector<thread> vhilos(n_hilos);
    // Variable compartida
    for (int i = 0; i < n_repeticion; ++i)
    {
        int x = 0;
        // Asignar la función a cada hilo
        for (auto &h: vhilos)
            h = thread(incrementar, ref(x));
        // Junto los hilos
        for (auto &h: vhilos)
            h.join();
        // Mostrar el valor final de x
        cout << x << " ";
    }
}

void ejemplo_2_race_condition_atomic() {
    // Cantidad de hilos
    int n_repeticion = 100;
    int n_hilos = 100;
    // Colección de hilos
    vector<thread> vhilos(n_hilos);
    // Variable compartida
    for (int i = 0; i < n_repeticion; ++i)
    {
        atomic<int> x = 0;
        // Asignar la función a cada hilo
        for (auto &h: vhilos)
            h = thread([&x]{ ++x; });
        // Junto los hilos
        for (auto &h: vhilos)
            h.join();
        // Mostrar el valor final de x
        cout << x << " ";
    }
}

class cuenta_bancaria {
    mutex mtx;
    double saldo = 0;
    condition_variable cv;
public:
    void deposito(double importe) {
        unique_lock ul(mtx);
        saldo += importe;
        ul.unlock();
        // Notifico el deposito (Ya deposite!!)
        cv.notify_one();
    }
    void retiro(double importe) {
        unique_lock ul(mtx);
        // Recibe la notificacion
        cv.wait(ul, [this, importe] { return this->saldo > importe; });
        if (saldo > importe)
            saldo -= importe;
        else
            cout << "Saldo insuficiente\n";
    }
    double get_saldo() { return saldo; }
};

void ejemplo_3_condition_variable() {
    int n_repeticiones = 100;
    for (int i = 0; i < n_repeticiones; ++i) {
        cuenta_bancaria cb;
        thread t1(&cuenta_bancaria::deposito, &cb, 100);
        thread t2(&cuenta_bancaria::retiro, &cb, 70);
        t1.join();
        t2.join();
        cout << cb.get_saldo() << " ";
    }
}


int main() {
//    ejemplo_1_race_condition_mutex();
//    ejemplo_2_race_condition_atomic();
    ejemplo_3_condition_variable();
    return 0;
}
