#include <iostream>
//#include <igraph/igraph_vector_pmt.h>
#include "./generator/make_graph.h"
#include "./generator/graph_generator.h"
#include "./generator/utils.h"
#include "./aml/aml.h"
#include "common.h"
#include "igraph/igraph.h"
//#include <igraph.h>

#define EDGE_FACTOR 16

void generate_graph(int scale, uint64_t seed1, int seed2, packed_edge** result) {
    double make_graph_start = MPI_Wtime();

    int64_t nglobaledges = (int64_t)(EDGE_FACTOR) << scale;
    int64_t nedges[1];

    make_graph(scale, nglobaledges, seed1, seed2, nedges, result);

    double make_graph_stop = MPI_Wtime();
    double make_graph_time = make_graph_stop - make_graph_start;
    if (rank == 0) {
        fprintf(stderr, "graph_generation:               %f s\n", make_graph_time);
    }

}

void generate_igraph(igraph_t *igraph, int scale, uint64_t seed1, int seed2) {
    igraph_vector_t edges_vector;

    // igraph use 2 item for an edge so we need 2x vector length
    int64_t nedges = (int64_t)(EDGE_FACTOR) << (scale + 1);
    packed_edge* result[1];

    generate_graph(scale, seed1, seed2, result);

    double make_graph_start = MPI_Wtime();

    igraph_vector_init(&edges_vector, nedges);

    for (int i = 0; i < nedges/2; ++i) {
        packed_edge edge = result[0][i];
        int64_t v0 = get_v0_from_edge(&edge);
        int64_t v1 = get_v1_from_edge(&edge);

        VECTOR(edges_vector)[i * 2] = v0;
        VECTOR(edges_vector)[i * 2 + 1] = v1;
    }

    igraph_bool_t directed = false;
    igraph_create(igraph, &edges_vector, 0, directed);

    double make_graph_stop = MPI_Wtime();
    double make_graph_time = make_graph_stop - make_graph_start;
    if (rank == 0) {
        fprintf(stderr, "igraph_generation:               %f s\n", make_graph_time);
    }
}

igraph_integer_t diameter(igraph_t *igraph){
    igraph_integer_t diameter;
    igraph_diameter(igraph, &diameter, 0, 0, 0, IGRAPH_UNDIRECTED, 1);
    return diameter;
}


int main(int argc, char** argv) {
    aml_init(&argc,&argv); //includes MPI_Init inside
    setup_globals();

    int SCALE = 8;
    uint64_t seed1 = 7, seed2 = 1;

    igraph_t graph;

    generate_igraph(&graph, SCALE, seed1, seed2);

    igraph_integer_t diameter = diameter(&graph);
    printf("Diameter of the graph: %d\n",
           (int) diameter);

    igraph_destroy(&graph);
    
    return 0;
}





