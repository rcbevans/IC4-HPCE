HPCE 2013 : CW3, Threaded Building Blocks
David Thomas, Imperial College

0. Overview
===========

This distribution contains a basic object framework for creating
and using fourier transforms, as well as two implementations:

1. A direct fourier transform taking O(n^2) steps.
2. A recursive fast-fourier transform taking O(n log n) steps.

Also included within the package is a very simple test suite
to check that the transforms work, and a registry which allows
new transforms to be added to the package.

Your job in this coursework is to explore a number of basic
(but effective) ways of using TBB to accelerate existing code,
and to compare and contrast the performance benefits of them.

1. Evironment and setup
=======================

Setting up TBB
--------------

You can download Threaded Building Blocks from the website:
https://www.threadingbuildingblocks.org/

The current stable version is 4.2, which is what I will be
using in the tests for the coursework. You have the choice
of downloading and building them from source (you have
knowledge of how to do that via configure/make), or you
can install the binary packages on your machine.

You have the choice of developing this under linux/posix
or windows. I don't really mind, as long as the code is
portable C++ (we won't have the problems from before with
sox, as there is no IO).

Included in this package is a makefile, which may get you
started in posix, and makefile.mk, which will allow you
to get started in windows with nmake (the make that comes
with visual studio). Note that nmake is much less powerfull
than GNU make, and cannot do many of the more advanced
things such as parallel build, or using eval to create
rules at make-time.

### Compiling in Windows

To use the visual studio version, open a visual studio command
prompt, then change to the directory where you downloaded
the code, and do:

    nmake -f makefile.mk all

the compiler will throw up errors due to missing tbb headers
and/or libraries - just modify makefile.mk to point at your
TBB install, and it should compile.

### General compilation notes

When I build your code, I will not be using your makefiles,
and will be compiling you .cpp files directly. There will
be a copy of TBB 4.2 available in the environment that your
code will use, but exactly how that happens is opaque to
you.

The fourier transform framework
-------------------------------

The package you have been given is a slightly overblown
framework for implementing fourier transforms. The file
`include\fourier_transform.hpp` defines a generic
fourier transform interface, allowing forwards and
backwards transforms.

In the src directory, there are two fourier transform
implementations, and two programs:

1. test_fourier_transform, which will run a number of
   simple tests on a given transform to check it works.
   The level of acceptable error in the results is
   defined as 1e-9 (which is quite high).
   
2. time_fourier_transform, which will time a given
   transform for increasing transform sizes with a given
   level of allowed parallelism.

If you build the two programs (using the appropriate
makefile, or some other mechanism), then you should
be able to test and time the two existing transforms.
For example, you can do:

    test_fourier_transform

which will list the two existing transforms, and:

    time_fourier_transform hpce.fast_fourier_transform

to see the effect of increasing transform size on
execution time (look in `time_fourier_transform.cpp` to
see what the columns mean).

Even though there is no parallelism in the default
framework, it still relies on TBB to control parallelism,
so it will not build, link, or execute, without TBB
being available.

Note: the direct fourier transform is very likely
to fail some tests, while the fast fourier transform
(should) pass all of them. This demonstrates that
even though two algorithms calculate the same result,
the order of calculation can change the numerical
accuracy significantly.

You may wish to explore the algorithmic tradeoffs
between the sequential direct and fast fourier
transforms - remember that parallelism and optimisation
are never a substitute for an algorithm with a better
intrinsic complexity.

2. Using tbb::parallel\_for in direct\_fourier\_transform
======================================================

The file `src/direct_fourier_transform.cpp` contains a classic
discrete fourier transform, which takes O(n^2) operations to
do an n-point fourier transform. The structure of this
code is essentially:

    for a=0:n-1
      out[a]=0;
      for b=0:n-1
        out[a]=out[a] + in[i] * exp(-i * 2*pi * a * b / n);
      end
    end

This should remind you of roots of unity and such-like
from way back when.

Overview of tbb::parallel_for
-----------------------------

There are two loops here, with a loop-carried dependency
through the inner loop, so a natural approach is to
try to parallelise over the outer loop. In matlab this
would be inefficient using parfor, except for vey large
transforms, but in TBB the overhead is fairly small.

The parfor equivalent is given by tbb::parallel_for,
which takes a functor (function-like object) and applies
it over a loop. The simplest approach is to take a
loop which looks like:

    for(unsigned i=0;i<n;i++){
      f(i);
    }

and turn it into:

    tbb::parallel_for(0u, n, f);

This approach works, but with C++11 lambda functions
we can do more sophisticated computation. So we can
take:

    for(unsigned i=0;i<n;i++){
      y[i]=f(x[i]);
    }

and turn it into:

    #include "tbb/parallel_for.h"

    tbb::parallel_for(0u, n, [=](unsigned i){
      y[i]=f(x[i]);
    });

Despite the way it initially appears, parallel_for is
still a normal function, but we just happen to be passing
in a variable which is code. The [=] syntax is introducing
a lambda, in a very similar way to the @ syntax in matlab,
so we could also do it in a much more matlab way:

    auto loop_body = [=](unsigned i){
      y[i]=f(x[i]);
    };
    tbb::parallel_for(0u, n, loop_body);

This is not the only form of parallel_for, but it is
the easiest and most direct. Other forms allows for
more efficient execution, but require more thought.

Using tbb::parallel_for in the fourier transform
------------------------------------------------

The framework is designed to support multiple fourier
transforms which you can select between, so we'll
need a way of distuinguishing your transform from
anyone elses (in principle I should be able to create
one giant executable containing everyone in the
class's transforms). The basic framework uses the namespace
`hpce`, but your classes will live in the namespace
`hpce::your_login`, and the source files in `src/your_login`.
For example, my namespace is `hpce::dt10`, and my
source files go in `src/dt10`.

There are three steps in this process:
1. Creating the new fourier transform class
2. Registering the class with the fourier framework
3. Adding the parallel constructs to the new class
4. Testing the parallel functionality
5. Finding the new execution time.

### Creating the new fourier transform class

Copy `src/direct_fourier_transform.cpp` into a new
file called `src/your_login/direct_fourier_transform_parfor.cpp`.
Modify the new file so that the contained class is called
`hpce::your_login::direct_fourier_transform_parfor`, and reports
`hpce.your_login.direct_fourier_transform_parfor` from `name()`. Apart
from renaming, you don't need to change any functionality yet.

To declare something in a nested namespace, simply
insert another namespace declaration inside the existing
one. For example, if you currently have `hpce::my_class`:

    namespace hpce{
	  class my_class{
	    ...
	  };
	};
	
you could get it into a new namespace called bobble, by
changing it to:

    namespace hpce{
	  namespace bobble{
	    class my_class{
		  ...
		};
	  };
	};
	
which would result in a class with the name `hpce::bobble::my_class`.

Add your new file to the set of objects (either by adding
it to FOURIER_IMPLEMENTATION_OBS in the makefile, or by
adding it to your visual studio project), and check
that it still compiles.

### Register the class with the fourier framework

As part of the modifications, you should have found
a function at the bottom of the file called `std::shared_ptr<fourier_transform> Create_direct_fourier_transform()`,
which you modified to `std::shared_ptr<fourier_transform> Create_direct_fourier_transform_parfor()`.
This is the factory function, used by the rest of the
framework to create transforms by name, without knowing
how they are implemented.

If you look in `src/fourier_transform_register_factories.cpp`, you'll
see a function called `fourier_transform::RegisterDefaultFactories`,
which is where you can register new transforms. To minimise
compile-time dependencies, the core framework knows nothing
about the transforms - all it knows is how to create them.

Towards the top is a space to declare your external factory
function, which can be uncommented. Then at the bottom
of `RegisterDefaultFactories`, uncomment the call which
registers the factory.

At this point, you should find that your new implementation
is listed if you build `test_fourier_transform` and do:

    test_fourier_transform hpce.your_login.direct_fourier_transform_parfor

Hopefully your implementation still works, as so far the
execution will be identical.

### Add the parallel_for loop

You need to rewrite the outer loop in both `forwards_impl` and `backwards_impl`,
using the transformation of for loop to `tbb::parallel_for` shown previously. I would
suggest doing one, running the tests, and then doing the other. You'll
need to make sure that you include the appropriate header for parallel_loop from
TBB at the top of the file, so that the function can be found. The linker path
will also need to be setup using whatever build tool you are using (e.g. the
settings in visual studio, or LDFLAGS in a makefile).

3. Using tbb::task_group in fast_fourier_transform
==================================================

The file `src/fast_fourier_transform.cpp` contains a radix-2
fast fourier transform, which takes O(n log n) operations to
do a transform. There are two main opportunities for parallelism
in this algorithm, both in `forwards_impl()`:
1. The recursive splitting, where the function makes two recursive
   calls to itself.
2. The iterative joining, where the results from the two recursive
   calls are joined together.

We will first exploit just the recursive splitting using
tbb::task_group.

Overview of tbb::task_group
---------------------------

Task groups allow use to specify sets of heterogenous
tasks that can run in parallel - by heterogenous, we
mean that each of the tasks can run different code and
perform a different function, unlike parallel_for where
one function is used for the whole iteration space.

Task groups are declared as an object on the stack:

    #include "tbb/task_group.h"

    tbb::task_group group;

you can then add tasks to the group dynamically,
using anything which looks like a nullary (zero-input)
function as the starting point:

    unsigned x=1, y=2;
    group.run( [&x](){ x=x*2; } );
	group.run( [&y](){ y=y+2; } );

After this code runs, we can't say anything about the
values of x and y, as each one has been captured by
reference but we don't know if they have been
modified yet. It is is possible that zero, one, or
both of the tasks have completed, so to rejoin the
tasks and synchronise we need to do:

    group.wait();

After this function executes, all tasks in the group
must have finished, so we know that x==2 and y==4.

### Create and register a new class 

Copy `src/fast_fourier_transform.cpp` into a new
file called `src/your_login/fast_fourier_transform_taskgroup.cpp`.
Modify the new file so that the contained class is called
`hpce::your_login::fast_fourier_transform_taskgroup`, and reports
`hpce.your_login.direct_fourier_transform_taskgroup` from name(). Apart
from renaming, you don't need to change any functionality yet.

As before, register the implementation with the implementation
in `src/fourier_transform_register_factories.cpp`, and check that
the transform still passes the test cases.

### Use tbb::task_group to parallelise the recursion

In the fast fourier transform there is a natural splitting
recursion in the section:

    forwards_impl(m,wn*wn,pIn,2*sIn,pOut,sOut);
    forwards_impl(m,wn*wn,pIn+sIn,2*sIn,pOut+sOut*m,sOut);

Modify the code to use tbb::task_group to turn the two
calls into child tasks. Don't worry about efficiency
yet, and keep splitting the tasks down to the point of
individual elements.

As before, test the implementation to make sure it still
works.

4. Using parallel iterations in the FFT
=======================================

Making the loop parallelisable
------------------------------

The FFT contains a for loop which at first glance appears to
be impossible to parallelise, due to the loop carried dependency
through w:

    std::complex<double> w=std::complex<double>(1.0, 0.0);

    for (size_t j=0;j<m;j++){
      std::complex<double> t1 = w*pOut[m+j];
      std::complex<double> t2 = pOut[j]-t1;
      pOut[j] = pOut[j]+t1;
      pOut[j+m] = t2;
      w = w*wn;
    }

However, we can in fact parallelise this loop as long as
we exploit some mathematical properties and batch things
carefully. On each iteration the loop calculates w=w*wn,
so if we look at the value of w at the start of each
iteration we have:

1. w=1*wn
2. w=1*wn*wn
3. w=1*wn*wn*wn

Generalising, we find that for iteration i, w=wn^i.

Hopefully it is obvious that raising something to the
power of i takes substantially less than i operations.
Try calculating (1+1e-8)^1e8 in matlab, and notice:

1. It is almost equal to _e_. Nice.
2. It clearly does not take anything like 1e8 operations to calculate.

In C++ the std::complex class supports the `std::pow` operator,
which will raise a complex number to a real scalar, which
can be used to jump ahead in the computation. In principle
we could use this property to make the loops completely
independent, but this will likely slow things down, as
powering is quite expensive (compared to one multiply). Instead we can use the idea
of _agglomeration_, which means instead of reducing the
code to the finest grain parallel tasks we can, we'll
group some of them into sequential tasks to reduce
overhead.

The overall pattern we want to use is to split the
single iteration space into two nested loops: an
outer one that can be parallelised, and an inner
sequential one:

    size_t K = something that divides m;
	
	for(size_t j0=0; j0<(m/K); j0++){
	  <Code to set w to the correct value for j=j0*K >
	  for (size_t j1=0; j1<K; j1++){
	    size_t j=j0*K+j1;  // Recover original loop variable
	    <Original loop body>
	  }
    }

Note that the outer loop has been collapsed down
to a contiguous loop counter (each j0 differs from
the next by one), so it is appropriate for use in
a parallel_for loop.

Also, the factor K is easy to choose in this case,
because m is always a power of two, so K can be
any power of two less than or equal to m (including
K=1).
	
### Create and register a new class 

Create a new class based on `src/fast_fourier_transform.cpp`, with:
- File name: `src/your_login/fast_fourier_transform_parfor.cpp`
- Class name: `hpce::your_login::fast_fourier_transform_parfor`
- Display name: `hpce.your_login.fast_fourier_transform_parfor`

This class should be based on the sequential version, not on the
task based version, so there is only one kind of parallelism.

### Apply the loop transformation

First apply the loop transformation described above,
without introducing any parallelism, and check it works
with various values of K. You want to do this step
first, so that you can test it deterministically.

Note that m gets smaller and smaller as it splits, so
you need to worry about m<K at the leaves of the
recursion. A simple solution is to use a guarded
version, such that if m<=K the original code is used,
and if m>K the new code is used.

An aside: you may find that the direct transform gets much
more accurate after this re-write, and now passes all
the tests with less than 1e-9 error. You might want to
think about where all that floating-point error is
creeping in.

### Introduce parallel_for

Convert the outer loop to a parallel_for loop. With the
loop already transformed and tested sequentially, this
should be trivial.

The value of K determines the balance between serial
work and the number of parallel jobs. For now, pick K=8,
as this will expose a lot of parallelism.

5. Combine both types of parallelism
====================================

We now have two types of parallelism that we know works,
so the natural choice is to combined them together.
Create a new implementation called `fast_fourier_transform_combined`,
using the conventions for naming from before, and integrate
both forms of parallelism.

6. Optimisation
===============

Our final implementation is a tweaking pass of the
parallelised version: we have introduced and exploited
as much parallelism as we can, but that introduces
overhead. We now want to balance parallelism versus
overhead.

Create a new implementation called `fast_fourier_transform_opt`,
and apply any optimisations you think reasonable (I'm not
talking about low-level tweaks, these should be low-hanging
fruit). Particular things to consider are:
- You may wish to stop creating seperate tasks during the
  splitting, once the sub-tasks drop below a certain size.
- The value of K should be chosen such that there is enough
  parallelism exposed to use multiple cores, but not so
  much that the cost of creating the parallel work becomes
  a limiting factor.
- There is some overhead involved in the serial version
  because it is radix2 (i.e. there is a base-case where n==2).
  You may want to look at stopping sequential splitting when n==4.

To re-iterate, the aim here is not low-level optimisation and
tweaking, it is for tuning and re-organising at a high-level,
while keeping reasonably readable and portable code. By applying
these techniqes, I got about a 6 times speed-up over the original
code using a 8-core (hyper-threaded) machine.

4. Measuring execution time and scalability
===========================================

The final task is to examine two different properties of the
system. Using whatever method you think appropriate
(matlab, excel, gnuplot, ...) create two pdfs, each containing
a single graph. The creation of the graphs does not need
to be automatic, but you need to be able to explain how
you made them if asked.

Your graphs need to be standalone, so they should contain any
key information needed to interpret or understand them in
the title (that doesn't mean huge amounts of info, but for
example, certain things like OS and processor type are useful
to know).

Speedup with transform size
---------------------------

One graph should be called `your_login_speedup.pdf`, and should
consider the speedup of the various implementations as a function
of n. This should consider speedup when all processors in the
system are available for use by TBB.

Scalability with processor count
--------------------------------

The other graph should be called `your_login_scalability.pdf`, and
should demonstrate _scalability_, which is the ability to usefully
employ extra processors. In a perfectly scalable system, if I use
P processors it should run P times faster than on 1 processor.

5. Submission
=============

Double-check your names all match up, as I'll be trying
to create your transforms both by direct instantiation,
and by pulling them out of the transform registry. Also,
don't forget that "your_login" does actually mean your
login needs to be substituted in wherever it is mentioned.

Zip your directory up, and upload it to blackboard. Don't worry about
cleaning any git files out, but take out any executables or object
files.

6. Errata
=========

None yet.


