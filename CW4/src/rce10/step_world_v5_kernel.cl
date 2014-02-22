enum cell_flags_t{
	Cell_Fixed		=0x1,
	Cell_Insulator	=0x2
};

__kernel void kernel_xy(const float inner,						//0
						const float outer,						//1
						__global const float *world_state,		//2
						__global const uint *world_properties,	//3
						__global float *buffer 					//4
						){

	uint x=get_global_id(0);
	uint y=get_global_id(1);
	uint w=get_global_size(0);

	uint index=y*w + x;

	uint myProps = world_properties[index];

	if((world_properties[index] & Cell_Fixed) || (world_properties[index] & Cell_Insulator)){
		// Do nothing, this cell never changes (e.g. a boundary, or an interior fixed-value heat-source)
		buffer[index]=world_state[index];
	}else{
		float contrib=inner;
		//float acc=inner*world_state[index];
		float acc = fma(inner, world_state[index], 0.0f);
		
		// Cell above
		if(myProps & 4) {
			contrib += outer;
			acc = fma(outer, world_state[index-w], acc);
			//acc += outer * world_state[index-w];
		}
		
		// Cell below
		if(myProps & 8) {
			contrib += outer;
			//acc += outer * world_state[index+w];
			acc = fma(outer, world_state[index+w], acc);
		}
		
		// Cell left
		if(myProps & 16) {
			contrib += outer;
			//acc += outer * world_state[index-1];
			acc = fma(outer, world_state[index-1], acc);
		}
		
		// Cell right
		if(myProps & 32) {
			contrib += outer;
			//acc += outer * world_state[index+1];
			acc = fma(outer, world_state[index+1], acc);
		}
		
		// Scale the accumulate value by the number of places contributing to it
		float res=acc/contrib;
		//float res = native_divide(acc, contrib);
		// Then clamp to the range [0,1]
		//res=min(1.0f, max(0.0f, res));
		res = clamp(res, 0.0f, 1.0f);
		buffer[index] = res;

	} // end of if(insulator){ ... } else {
}	