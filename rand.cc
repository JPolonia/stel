
#include <random>
#include <chrono>

extern "C" {

    double urng(void) {
        std::mt19937_64 rng;
        // initialize the random number generator with time-dependent seed
        uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
        rng.seed(ss);
        // initialize a uniform distribution between 0 and 1
        std::uniform_real_distribution<double> unif(0, 1);
        return unif(rng);
    }
}