#include "cpp-mpi-programs/ring.cpp"
#include <boost/mpi.hpp>
#include <iostream>
#include <mpi.h>
#include <string>
namespace mpi = boost::mpi;

int main() {
    // communicate();
    ring();
    return 0;
}
