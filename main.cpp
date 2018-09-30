#include <iostream>
#include "./generator/make_graph.h"
#include "./generator/graph_generator.h"
#include "./generator/utils.h"
#include "./aml/aml.h"
#include "common.h"
#include "igraph/igraph.h"
//#include <igraph.h>



int main(int argc, char** argv) {

    aml_init(&argc,&argv); //includes MPI_Init inside
    setup_globals();


    int SCALE = 12;
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


    igraph_integer_t diameter;
    igraph_t graph;
    igraph_rng_seed(igraph_rng_default(), 42);
    igraph_erdos_renyi_game(&graph, IGRAPH_ERDOS_RENYI_GNP, 1000, 5.0/1000,
                            IGRAPH_UNDIRECTED, IGRAPH_NO_LOOPS);
    igraph_diameter(&graph, &diameter, 0, 0, 0, IGRAPH_UNDIRECTED, 1);
    printf("Diameter of a random graph with average degree 5: %d\n",
           (int) diameter);
    igraph_destroy(&graph);

    return 0;
}



