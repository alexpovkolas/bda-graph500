#include <iostream>
#include "./generator/make_graph.h"
#include "./generator/graph_generator.h"
#include "./generator/utils.h"
#include "./aml/aml.h"
#include "common.h"
#include "igraph/igraph.h"
#include <random>
#include <map>


#define EDGE_FACTOR 16
#define SCALE 8
#define TESTS_AMOUNT 2
#define SEED 777
#define DIRECTED false

using namespace std;

void generate_graph(int scale, uint64_t seed1, int seed2, packed_edge** result) {
    double make_graph_start = MPI_Wtime();

    int64_t nglobaledges = (int64_t) (EDGE_FACTOR) << scale;
    int64_t nedges[1];

    make_graph(scale, nglobaledges, seed1, seed2, nedges, result);

    double make_graph_stop = MPI_Wtime();
    double make_graph_time = make_graph_stop - make_graph_start;

    fprintf(stderr, "graph_generation:               %f s\n", make_graph_time);
}

void generate_igraph(igraph_t *igraph, int scale, uint64_t seed1, int seed2) {
    igraph_vector_t edges_vector;

    // igraph use 2 item for an edge so we need 2x vector length
    int64_t nedges = (int64_t)(EDGE_FACTOR) << (scale + 1);
    packed_edge* result;

    generate_graph(scale, seed1, seed2, &result);

    double make_graph_start = MPI_Wtime();

    igraph_vector_init(&edges_vector, nedges);

    for (int i = 0; i < nedges/2; ++i) {
        packed_edge edge = result[i];
        int64_t v0 = get_v0_from_edge(&edge);
        int64_t v1 = get_v1_from_edge(&edge);

        VECTOR(edges_vector)[i * 2] = v0;
        VECTOR(edges_vector)[i * 2 + 1] = v1;
    }

    free(result);

    igraph_create(igraph, &edges_vector, 0, IGRAPH_DIRECTED);

    double make_graph_stop = MPI_Wtime();
    double make_graph_time = make_graph_stop - make_graph_start;

    fprintf(stderr, "igraph_generation:               %f s\n", make_graph_time);
}

igraph_integer_t diameter(igraph_t *igraph){
    igraph_integer_t diameter;
    igraph_diameter(igraph, &diameter, 0, 0, 0, DIRECTED ? IGRAPH_DIRECTED : IGRAPH_UNDIRECTED, 1);
    return diameter;
}

igraph_real_t avg_distance(igraph_t *igraph){
    igraph_real_t result;
    igraph_average_path_length(igraph, &result, DIRECTED ? IGRAPH_DIRECTED : IGRAPH_UNDIRECTED, 1);
    return result;
}

map<int, double> hist(igraph_t *igraph){
    igraph_vector_t v;

    igraph_vector_init(&v, 0);

    int result = igraph_degree(igraph, &v, igraph_vss_all(), IGRAPH_ALL, IGRAPH_LOOPS);

    map<int, int> frequencies;
    for (long i=0; i<igraph_vector_size(&v); i++) {
        int degree = (long int) VECTOR(v)[i];
        frequencies[degree]++;
    }

    igraph_integer_t nedges = igraph_ecount(igraph);
    map<int, double> hist;
    for(auto it = frequencies.cbegin(); it != frequencies.cend(); ++it) {
        hist[it->first] = it->second / (double) nedges;
    }

    return hist;
}


int main(int argc, char** argv) {
    aml_init(&argc,&argv); //includes MPI_Init inside
    setup_globals();

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,6); // distribution in range [1, 6]

    double tests_start = MPI_Wtime();

    int scale = SCALE;
    int test_amount = TESTS_AMOUNT;
    int seed = dist6(rng);

    int diameter_sum = 0;
    double distance_sum = 0;


    map<int, double> histogram;
    for (int k = 0; k < test_amount; ++k) {
        uint64_t seed1 = k * seed, seed2 = k + seed;

        igraph_t graph;
        generate_igraph(&graph, scale, seed1, seed2);


        diameter_sum += diameter(&graph);
        distance_sum += avg_distance(&graph);
        map<int, double> hist_map = hist(&graph);

        for(auto it = hist_map.cbegin(); it != hist_map.cend(); ++it) {
            cout << it->first << " " << it->second << "\n";
        }

        igraph_destroy(&graph);
    }

    double avg_diameter = diameter_sum  / (double)test_amount;
    double avg_distance = distance_sum  / (double)test_amount;

    printf("Avg diameter of the graph: %f\n",
           avg_diameter);

    printf("Avg distance of the graph: %f\n",
           avg_distance);


    double tests_stop = MPI_Wtime();
    double tests_time = tests_stop - tests_start;

    fprintf(stderr, "running time:               %f s\n", tests_time);


//    igraph_t g;
//    igraph_vector_t v, seq;
//    int ret;
//    igraph_integer_t mdeg, nedges;
//    long int i;
//    long int ndeg;
//
//    /* Create graph */
//    igraph_vector_init(&v, 8);
//    VECTOR(v)[0]=0; VECTOR(v)[1]=1;
//    VECTOR(v)[2]=1; VECTOR(v)[3]=2;
//    VECTOR(v)[4]=2; VECTOR(v)[5]=3;
//    VECTOR(v)[6]=2; VECTOR(v)[7]=2;
//    igraph_create(&g, &v, 0, IGRAPH_DIRECTED);
//
//    igraph_degree(&g, &v, igraph_vss_all(), IGRAPH_ALL, IGRAPH_LOOPS);
//    print_vector(&v, stdout);


    return 0;
}





