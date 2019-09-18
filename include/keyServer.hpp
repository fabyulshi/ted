#ifndef GENERALDEDUPSYSTEM_KEYSERVER_HPP
#define GENERALDEDUPSYSTEM_KEYSERVER_HPP

#include "configure.hpp"
#include "cryptoPrimitive.hpp"
#include "dataStructure.hpp"
#include "messageQueue.hpp"
#include "openssl/bn.h"
#include "optimalSolver.hpp"
#include "socket.hpp"
#include <bits/stdc++.h>

#define KEYMANGER_PRIVATE_KEY "key/server.key"

class keyServer {
private:
    u_int** sketchTable_;
    uint64_t sketchTableCounter_;
    double T_;
    bool opSolverFlag_;
    vector<pair<string, int>> opInput_;
    int opm_;
    uint64_t sketchTableWidith_;
    std::mutex multiThreadEditTMutex_;
    std::mutex multiThreadEditSketchTableMutex_;
    random_device rd_;
    mt19937_64 gen_;
    u_char keyServerPrivate_[64];
    int optimalSolverComputeItemNumberThreshold_;

public:
    keyServer();
    ~keyServer();
    void runKeyGen(Socket socket);
    void runOptimalSolver();
};

#endif //GENERALDEDUPSYSTEM_KEYSERVER_HPP
