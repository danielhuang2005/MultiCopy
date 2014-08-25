# Включаем профайлер gprof
QMAKE_CFLAGS   += -pg
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS   += -pg

# Включаем профайлер gcov
QMAKE_CFLAGS   += -fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LFLAGS   += -fprofile-arcs -ftest-coverage
