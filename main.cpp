#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#include <cxxabi.h>
#include <memory>
static std::string demangle(const char* name) {
    auto demangledName = abi::__cxa_demangle(name, 0, 0, nullptr);
    auto result = std::string(demangledName);
    free(demangledName);
    return result;
}
#define DEMANGLE(CLASS) demangle(CLASS)
#elif defined(_MSC_VER)
#define DEMANGLE(CLASS) CLASS
#endif
///
#include <unistd.h>
#include <sys/times.h>

using ClockType = decltype(times(nullptr));

struct tms start, end;
ClockType clock_start, clock_end;

#define START_TIME() \
clock_start = times(&start);

#define END_TIME() \
clock_end = times(&end);

#define DELTA_TIME() \
(double(end.tms_utime - start.tms_utime) / sysconf(_SC_CLK_TCK)) 
//

enum class JMP {
    RANDOM,
    EVEN_ODD,
    MINIMIZE
};

const std::size_t count_epoch = 10000000;

template<JMP type = JMP::RANDOM>
auto test() -> decltype(DELTA_TIME()) {
    static bool jmps[count_epoch];
    for (size_t i = 0; i < count_epoch; ++i) {
        jmps[i] = random() % 2;
    }

    START_TIME()
    for (size_t i = 0; i < count_epoch; ++i) {
        if (jmps[i]) {
            volatile int a = 10;
            a += 20;
        } else {
            volatile int a = 20;
            a += 20;
        }
    }
    END_TIME()

    return DELTA_TIME();
}

template<>
auto test<JMP::EVEN_ODD>() -> decltype(DELTA_TIME()) {
    static bool jmps[count_epoch];
    for (size_t i = 0; i < count_epoch; ++i) {
        jmps[i] = i % 2;
    }

    START_TIME()
    for (size_t i = 0; i < count_epoch; ++i) {
        if (jmps[i]) {
            volatile int a = 10;
            a += 20;
        } else {
            volatile int a = 20;
            a += 20;
        }
    }
    END_TIME()

    return DELTA_TIME();
}

size_t SEED = 0;

template<>
auto test<JMP::MINIMIZE>() -> decltype(DELTA_TIME()) {
    static bool jmps[count_epoch];
    for (size_t i = 0; i < count_epoch; ++i) {
        jmps[i] = random() >> (i % 199) & 1;
    }

    START_TIME()
    for (size_t i = 0; i < count_epoch; ++i) {
        if (jmps[i]) {
            volatile int a = 10;
            a += 20;
        } else {
            volatile int a = 20;
            a += 20;
        }
    }
    END_TIME()

    return DELTA_TIME();
}

#include <iostream>
template<JMP jmp>
void benchmark() {
    using delta_t = decltype(DELTA_TIME());
    
    std::string name;
    if constexpr (jmp == JMP::RANDOM) {
        name = "random";
    }
    if constexpr (jmp == JMP::EVEN_ODD) {
        name = "even/odd";
    }
    if constexpr (jmp == JMP::MINIMIZE) {
        name = "minimize";
    }

    auto timediff = test<jmp>();

    std::cout << "[" << count_epoch << "][" << name << "]: " << timediff << ", s" << std::endl;
}

int main() {
    benchmark<JMP::RANDOM>();
    benchmark<JMP::EVEN_ODD>();
    srand(16);
    benchmark<JMP::MINIMIZE>();
    return 0;
}