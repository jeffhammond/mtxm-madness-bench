CXX = /opt/intel/composer_xe_2015.3.187/bin/intel64/icpc
CXXFLAGS  = -O3 -std=c++11 -qopenmp -mavx
CXXFLAGS += -qopt-report=5

INCLUDE = -I.
CXXFLAGS += $(INCLUDE)

BLAS = -mkl=sequential
LIBS = 	$(BLAS)

all: test_mtxmq.x test_Zmtxmq.x

test_mtxmq.x: test_mtxmq.cc mtxmq.h
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

test_Zmtxmq.x: test_Zmtxmq.cc mtxmq.h
	$(CXX) $(CXXFLAGS) $< $(LIBS) -o $@

clean:
	-rm -f test_mtxmq.x test_Zmtxmq.x
	-rm -rf test_mtxmq.x.dSYM test_Zmtxmq.x.dSYM
	-rm -rf test_mtxmq.optrpt test_Zmtxmq.optrpt

