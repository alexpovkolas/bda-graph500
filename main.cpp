#include <iostream>
#include "./generator/make_graph.h"
#include "./generator/graph_generator.h"
#include "./generator/utils.h"
#include "./aml/aml.h"
#include "common.h"


int main(int argc, char** argv) {

    aml_init(&argc,&argv); //includes MPI_Init inside
    setup_globals();


    int SCALE = 8;
    int edgefactor = 16;

    int64_t nedges[1];
    packed_edge* result[1];
    int64_t nglobaledges = (int64_t)(edgefactor) << SCALE;
    uint64_t seed1 = 2, seed2 = 3;

    double make_graph_start = MPI_Wtime();

    make_graph(SCALE, nglobaledges, seed1, seed2, nedges, result);

    double make_graph_stop = MPI_Wtime();
    double make_graph_time = make_graph_stop - make_graph_start;
    if (rank == 0) {
        fprintf(stderr, "graph_generation:               %f s\n", make_graph_time);
    }

    // example of using
    
//    for (int i = 0; i < *nedges; ++i) {
//        packed_edge edge = result[0][i];
//        int64_t v0 = get_v0_from_edge(&edge);
//        int64_t v1 = get_v1_from_edge(&edge);
//        int64_t brpoint = v0 + v1;
//    }

    return 0;
}



