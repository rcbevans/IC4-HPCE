#include "fourier_transform.hpp"

#include <cmath>
#include <cassert>
#include <thread>
#include "tbb/task_group.h"
#include "tbb/parallel_for.h"

namespace hpce
{

	namespace rce10
			{

		class fast_fourier_transform_opt
			: public fourier_transform
		{
		protected:
			/* Standard radix-2 FFT only supports binary power lengths */
			virtual size_t calc_padded_size(size_t n) const
			{
				assert(n!=0);
				
				size_t ret=1;
				while(ret<n){
					ret<<=1;
				}
				
				return ret;
			}

			virtual void forwards_impl(
				size_t n,	const std::complex<double> &wn,
				const std::complex<double> *pIn, size_t sIn,
				std::complex<double> *pOut, size_t sOut
			) const 
			{
				assert(n>0);
				
				if (n == 1){
					pOut[0] = pIn[0];
				}else if (n == 2){
					pOut[0] = pIn[0]+pIn[sIn];
					pOut[sOut] = pIn[0]-pIn[sIn];
				}else{

					size_t m = n/2;

					size_t group_size = 1 << 7;

					if (n > group_size){
						tbb::task_group group;

						group.run([&]{forwards_impl(m,wn*wn,pIn,2*sIn,pOut,sOut);});
						group.run([&]{forwards_impl(m,wn*wn,pIn+sIn,2*sIn,pOut+sOut*m,sOut);});
						 
						group.wait();
					} else {
						forwards_impl(m,wn*wn,pIn,2*sIn,pOut,sOut);
						forwards_impl(m,wn*wn,pIn+sIn,2*sIn,pOut+sOut*m,sOut);
					}

					size_t K = 1 << 8;

					if (m > K){
						auto forwards_impl_loop = [=](size_t j0){
							std::complex<double> w=std::pow(wn, j0*K);
							size_t j = 0;
							std::complex<double> t1 = 0;
							std::complex<double> t2 = 0;
							for (size_t j1=0; j1<K; j1++){
								j=j0*K+j1;  // Recover original loop variable
								t1 = w*pOut[m+j];
								t2 = pOut[j]-t1;
								pOut[j] = pOut[j]+t1;                 /*  pOut[j] = pOut[j] + w^i pOut[m+j] */
								pOut[j+m] = t2;                          /*  pOut[j] = pOut[j] - w^i pOut[m+j] */
								w = w*wn;
							}
						};
						tbb::parallel_for<size_t>(0,(m/K), forwards_impl_loop);
					} else {
						std::complex<double> w=std::complex<double>(1.0, 0.0);
						std::complex<double> t1 = 0;
						std::complex<double> t2 = 0;
						for (size_t j=0;j<m;j++){
							t1 = w*pOut[m+j];
							t2 = pOut[j]-t1;
							pOut[j] = pOut[j]+t1;                 /*  pOut[j] = pOut[j] + w^i pOut[m+j] */
							pOut[j+m] = t2;                          /*  pOut[j] = pOut[j] - w^i pOut[m+j] */
							w = w*wn;
						}
					}
				}
			}
			
			virtual void backwards_impl(
				size_t n,	const std::complex<double> &wn,
				const std::complex<double> *pIn, size_t sIn,
				std::complex<double> *pOut, size_t sOut
			) const 
			{
				forwards_impl(n, 1.0/wn, pIn, sIn, pOut, sOut);
				
				for(size_t i=0;i<n;i++){
					pOut[i]=pOut[i]/(double)n;
				}
			}
			
		public:
			virtual std::string name() const
			{ return "hpce.rce10.fast_fourier_transform_opt"; }
			
			virtual bool is_quadratic() const
			{ return false; }
		};

		std::shared_ptr<fourier_transform> Create_fast_fourier_transform_opt()
		{
			return std::make_shared<fast_fourier_transform_opt>();
		}
	}//namespace rce10
}; // namespace hpce
