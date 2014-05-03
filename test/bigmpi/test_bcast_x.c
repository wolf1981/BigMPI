#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <strings.h>

#include <mpi.h>
#include "bigmpi.h"
#include "verify_buffer.h"

#ifdef BIGMPI_MAX_INT
const MPI_Count test_int_max = BIGMPI_MAX_INT;
#else
#include <limits.h>
const MPI_Count test_int_max = INT_MAX;
#endif

/* Yes, it is technically unsafe to cast MPI_Count to MPI_Aint or size_t without checking,
 * given that MPI_Count might be 128b and MPI_Aint and size_t might be 64b, but BigMPI
 * does not aspire to support communication of more than 8 EiB messages at a time.
 * If your system has more than 8 EiB of memory, please contact me, as I would like to use it :-) */

int main(int argc, char * argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size<2) {
        printf("Use 2 or more processes. \n");
        MPI_Finalize();
        return 1;
    }

    int l = (argc > 1) ? atoi(argv[1]) : 2;
    int m = (argc > 2) ? atoi(argv[2]) : 17777;
    MPI_Count n = l * test_int_max + m;

    char * buf = NULL;

    MPI_Alloc_mem((MPI_Aint)n, MPI_INFO_NULL, &buf);

    memset(buf, rank, (size_t)n);

    /* collective communication */
    MPIX_Bcast_x(buf, n, MPI_CHAR, 0 /* root */, MPI_COMM_WORLD);

    verify_buffer(buf, n, 0);

    MPI_Free_mem(buf);

    if (rank==0) {
        printf("SUCCESS\n");
    }

    MPI_Finalize();

    return 0;
}
