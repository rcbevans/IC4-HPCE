# Note on old compilers  *cough*  DoC  *cough* you might need -std=c++0x instead
CPPFLAGS = -I include -Wall -std=c++11
LDFLAGS = 
LDLIBS = -lm -lOpenCL

# Turn on optimisations
CPPFLAGS += -O2
# CPPFLAGS += -msse2 -ffloat-store

HEAT_CORE = src/heat.o

bin/make_world : src/make_world.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/step_world : src/step_world.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/render_world : src/render_world.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/step_world_v1_lambda : src/rce10/step_world_v1_lambda.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/step_world_v2_function : src/rce10/step_world_v2_function.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/step_world_v3_opencl : src/rce10/step_world_v3_opencl.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/step_world_v4_double_buffered : src/rce10/step_world_v4_double_buffered.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

bin/step_world_v5_packed_properties : src/rce10/step_world_v5_packed_properties.cpp $(HEAT_CORE)
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
	
bin/test : src/test_opencl.cpp
	-mkdir -p bin
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

all : bin/test bin/step_world bin/make_world bin/step_world bin/render_world \
	bin/step_world_v1_lambda bin/step_world_v2_function bin/step_world_v3_opencl \
	bin/step_world_v4_double_buffered bin/step_world_v5_packed_properties

clean : 
	rm bin/* src/*.o