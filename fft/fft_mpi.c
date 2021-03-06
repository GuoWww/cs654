#include <fftw3-mpi.h>
#include <sys/time.h>
#include <stdlib.h>

double getTime() {
	struct timeval thetime;
	gettimeofday( &thetime, 0 );
	return thetime.tv_sec + thetime.tv_usec / 1000000.0;
}


int main(int argc, char **argv) {
	fftwf_plan plan;
	fftwf_complex *data;
	ptrdiff_t alloc_local, local_n0, local_0_start, i, j;

	if (argc != 2) { printf("usage: ./fft_mpi MATRIX_SIZE\n"); exit(1); }

	const ptrdiff_t N0 = atoi(argv[1]);
	const ptrdiff_t N1 = N0;
	int id;
	double startTime, totalTime;
	totalTime = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	fftwf_mpi_init();

	/* get local data size and allocate */
	alloc_local = fftwf_mpi_local_size_2d(N0, N1, MPI_COMM_WORLD, &local_n0, &local_0_start);
	data = fftwf_alloc_complex(alloc_local);//(fftwf_complex *) fftwf_malloc(sizeof(fftw_complex) * alloc_local);

	/* create plan for in-place forward DFT */
	plan = fftwf_mpi_plan_dft_2d(N0, N1, data, data, MPI_COMM_WORLD, FFTW_FORWARD, FFTW_ESTIMATE);

	/* initialize data to some function my_function(x,y) */
	for (i = 0; i < local_n0; ++i) for (j = 0; j < N1; ++j){
		data[i*N1 + j][0] = local_0_start;;//my_function(local_0_start + i, j);
		data[i*N1 + j][1]=i;
	}

	/* compute transforms, in-place, as many times as desired */
	
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 0) {
		startTime = getTime();
	}
	fftwf_execute(plan);
	MPI_Barrier(MPI_COMM_WORLD);
	if (id == 0) {
		totalTime += getTime() - startTime;
	}

	fftwf_destroy_plan(plan);
	fftwf_mpi_cleanup();

	if (id == 0) {
		printf("%.5f\n", totalTime);
	}

	MPI_Finalize();
	return 0;
}
