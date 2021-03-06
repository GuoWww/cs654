//-lfftw3_omp -lfftw3 -lm
 
#include <fftw3.h>
#include <math.h>
#include <omp.h> 
#include <sys/time.h>
#include <stdlib.h>

double getTime() {
    timeval thetime;
    gettimeofday( &thetime, 0 );
    return thetime.tv_sec + thetime.tv_usec / 1000000.0;
}


int main(int argc, char **argv) {

    if (argc != 3) { printf("usage: ./fft_omp MATRIX_SIZE NUM_THREADS\n"); exit(1); }

    omp_set_num_threads(atoi(argv[2])); //before the thing below it
    const ptrdiff_t N0 = atoi(argv[1]);
    const ptrdiff_t N1 = N0;
    double startTime, totalTime;
    totalTime = 0;

    int fftwf_init_threads(void); //before calling any fftw routine
    fftwf_plan plan;
    fftwf_complex *data;
 
    data = (fftwf_complex *) fftwf_malloc(sizeof(fftwf_complex) * N0 * N1);
 
    fftwf_plan_with_nthreads(omp_get_max_threads()); //before creating a plan
    /* create plan for forward DFT */
    plan = fftwf_plan_dft_2d(N0, N1, data, data, FFTW_FORWARD, FFTW_ESTIMATE);
 
    /* initialize data to some function my_function(x,y) */
    int i, j;
    double pdata=0;
    for (i = 0; i < N0; ++i) {
        for (j = 0; j < N1; ++j) {
          data[i*N1+j][0] = i; 
          data[i*N1+j][1] = 0;
          pdata += data[i*N1+j][0] * data[i*N1+j][0] + data[i*N1+j][1] * data[i*N1+j][1];
        }
    }
    // printf("power of original data is %f\n", pdata);
 
    /* compute transforms, in-place, as many times as desired */
    startTime = getTime();
    fftwf_execute(plan);
    totalTime += getTime() - startTime;
 
    double normalization = sqrt((double)N0*N1);
    double ptransform = 0;
 
    /*normalize data and calculate power of transform */
    for (i = 0; i < N0; ++i){
        for (j = 0; j < N1; ++j){
          data[i*N1+j][0] /= normalization;
          data[i*N1+j][1] /= normalization;
          ptransform += data[i*N1+j][0] * data[i*N1+j][0] + data[i*N1+j][1] * data[i*N1+j][1];
        }
    }
 
    // printf("power of transform is %f\n", pdata);
  
    fftwf_destroy_plan(plan);
    fftwf_free(data);

    printf("%.5lf\n", totalTime);
    return 0;
}
