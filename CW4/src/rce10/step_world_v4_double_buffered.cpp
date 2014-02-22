#include "heat.hpp"

#include <stdexcept>
#include <cmath>
#include <cstdint>
#include <memory>
#include <cstdio>

#define __CL_ENABLE_EXCEPTIONS 
#include "CL/cl.hpp"

#include <fstream>
#include <streambuf>

namespace hpce{
	namespace rce10{

		void kernel_xy(const uint32_t x, const uint32_t y, const uint32_t w, const float inner, const float outer, const float *world_state, const uint32_t *world_properties, float *buffer){
			uint32_t index=y*w + x;
			if((world_properties[index] & Cell_Fixed) || (world_properties[index] & Cell_Insulator)){
				// Do nothing, this cell never changes (e.g. a boundary, or an interior fixed-value heat-source)
				buffer[index]=world_state[index];
			}else{
				float contrib=inner;
				float acc=inner*world_state[index];
				
				// Cell above
				if(! (world_properties[index-w] & Cell_Insulator)) {
					contrib += outer;
					acc += outer * world_state[index-w];
				}
				
				// Cell below
				if(! (world_properties[index+w] & Cell_Insulator)) {
					contrib += outer;
					acc += outer * world_state[index+w];
				}
				
				// Cell left
				if(! (world_properties[index-1] & Cell_Insulator)) {
					contrib += outer;
					acc += outer * world_state[index-1];
				}
				
				// Cell right
				if(! (world_properties[index+1] & Cell_Insulator)) {
					contrib += outer;
					acc += outer * world_state[index+1];
				}
				
				// Scale the accumulate value by the number of places contributing to it
				float res=acc/contrib;
				// Then clamp to the range [0,1]
				res=std::min(1.0f, std::max(0.0f, res));
				buffer[index] = res;

			} // end of if(insulator){ ... } else {
		}

		std::string LoadSource(const char *fileName)
		{
			// Don't forget to change your_login here
			std::string baseDir="src/rce10";
			if(getenv("HPCE_CL_SRC_DIR")){
				baseDir=getenv("HPCE_CL_SRC_DIR");
			}
			
			std::string fullName=baseDir+"/"+fileName;
			
			std::ifstream src(fullName, std::ios::in | std::ios::binary);
			if(!src.is_open())
				throw std::runtime_error("LoadSource : Couldn't load cl file from '"+fullName+"'.");
			
			return std::string(
				(std::istreambuf_iterator<char>(src)), // Node the extra brackets.
	            std::istreambuf_iterator<char>()
			);
		}
			
		void StepWorldV3OpenCL(world_t &world, float dt, unsigned n)
		{
			//Initialize CL
			std::vector<cl::Platform> platforms;
				
			cl::Platform::get(&platforms);
			if(platforms.size()==0)
				throw std::runtime_error("No OpenCL platforms found.");

			std::cerr<<"Found "<<platforms.size()<<" platforms\n";
			for(unsigned i=0;i<platforms.size();i++){
				std::string vendor=platforms[i].getInfo<CL_PLATFORM_VENDOR>();
				std::cerr <<"  Platform "<< i <<" : "<< vendor << std::endl;
			}

			int selectedPlatform=0;
			if(getenv("HPCE_SELECT_PLATFORM")){
				selectedPlatform=atoi(getenv("HPCE_SELECT_PLATFORM"));
			}
			std::cerr << "Choosing platform " << selectedPlatform << ", " << platforms[selectedPlatform].getInfo<CL_PLATFORM_VENDOR>() << std::endl;
			cl::Platform platform=platforms.at(selectedPlatform);

			std::vector<cl::Device> devices;
			platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);	
			if(devices.size()==0){
				throw std::runtime_error("No opencl devices found.\n");
			}
				
			std::cerr<<"Found "<<devices.size()<<" devices" << std::endl;;
			for(unsigned i=0;i<devices.size();i++){
				std::string name=devices[i].getInfo<CL_DEVICE_NAME>();
				std::cerr<<"  Device "<<i<<" : "<<name<<std::endl;
			}

			int selectedDevice=0;
			if(getenv("HPCE_SELECT_DEVICE")){
				selectedDevice=atoi(getenv("HPCE_SELECT_DEVICE"));
			}
			std::cerr<<"Choosing device " << selectedDevice << ", " << devices[selectedDevice].getInfo<CL_DEVICE_NAME>() << std::endl;
			cl::Device device=devices.at(selectedDevice);

			cl::Context context(devices);

			std::string kernelSource=LoadSource("step_world_v3_kernel.cl");

			cl::Program::Sources sources;	// A vector of (data,length) pairs
			sources.push_back(std::make_pair(kernelSource.c_str(), kernelSource.size()+1));	// push on our single string
			
			cl::Program program(context, sources);
			try{
				program.build(devices);
			}catch(...){
				for(unsigned i=0;i<devices.size();i++){
					std::cerr<<"Log for device "<<devices[i].getInfo<CL_DEVICE_NAME>()<<":\n\n";
					std::cerr<<program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i])<<"\n\n";
				}
				throw;
			}

			size_t cbBuffer=4*world.w*world.h;
			cl::Buffer buffProperties(context, CL_MEM_READ_ONLY, cbBuffer);
			cl::Buffer buffState(context, CL_MEM_READ_WRITE, cbBuffer);
			cl::Buffer buffBuffer(context, CL_MEM_READ_WRITE, cbBuffer);

			cl::Kernel kernel(program, "kernel_xy");

			cl::CommandQueue queue(context, device);

			// END CL INITIALIZATION

			uint32_t w=world.w, h=world.h;
			
			float outer=world.alpha*dt;		// We spread alpha to other cells per time
			float inner=1-outer/4;				// Anything that doesn't spread stays
			
			kernel.setArg(0, inner);
			kernel.setArg(1, outer);
			kernel.setArg(2, buffState);
			kernel.setArg(3, buffProperties);
			kernel.setArg(4, buffBuffer);

			queue.enqueueWriteBuffer(buffProperties, CL_TRUE, 0, cbBuffer, &world.properties[0]);
			queue.enqueueWriteBuffer(buffState, CL_TRUE, 0, cbBuffer, &world.state[0]);

			for(unsigned t=0;t<n;t++){

				if(!(t%2)){
					kernel.setArg(2, buffState);
					kernel.setArg(4, buffBuffer);
				} else {
					kernel.setArg(4, buffState);
					kernel.setArg(2, buffBuffer);
				}

				cl::NDRange offset(0, 0);				// Always start iterations at x=0, y=0
				cl::NDRange globalSize(h, w);	// Global size must match the original loops
				cl::NDRange localSize=cl::NullRange;	// We don't care about local size

				queue.enqueueNDRangeKernel(kernel, offset, globalSize, localSize);
						
				queue.enqueueBarrier();	// <- new barrier here
				
				// std::swap(buffState, buffBuffer);
			
				world.t += dt; // We have moved the world forwards in time
				
			} // end of for(t...

			queue.enqueueReadBuffer(buffState, CL_TRUE, 0, cbBuffer, &world.state[0], NULL);

		}

	} //namespace rce10
	
}; // namepspace hpce

int main(int argc, char *argv[])
{
	float dt=0.1;
	unsigned n=1;
	bool binary=false;
	
	if(argc>1){
		dt=strtof(argv[1], NULL);
	}
	if(argc>2){
		n=atoi(argv[2]);
	}
	if(argc>3){
		if(atoi(argv[3]))
			binary=true;
	}
	
	try{
		hpce::world_t world=hpce::LoadWorld(std::cin);
		std::cerr<<"Loaded world with w="<<world.w<<", h="<<world.h<<std::endl;
		
		std::cerr<<"Stepping by dt="<<dt<<" for n="<<n<<std::endl;
		hpce::rce10::StepWorldV3OpenCL(world, dt, n);
		
		hpce::SaveWorld(std::cout, world, binary);
	}catch(const std::exception &e){
		std::cerr<<"Exception : "<<e.what()<<std::endl;
		return 1;
	}
		
	return 0;
}