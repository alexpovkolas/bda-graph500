#include <iostream>
#include "./generator/make_graph.h"
#include "./generator/graph_generator.h"
#include "./generator/utils.h"
#include "./aml/aml.h"
#include "common.h"
#include "igraph/igraph.h"
#include <random>
#include <map>
#include <fstream>



#define EDGE_FACTOR 16
#define SCALE 12
#define TESTS_AMOUNT 1000
#define SEED 777
#define DIRECTED true

using namespace std;

void generate_graph(int scale, uint64_t seed1, int seed2, packed_edge** result) {
    int64_t nglobaledges = (int64_t) (EDGE_FACTOR) << scale;
    int64_t nedges[1];

    make_graph(scale, nglobaledges, seed1, seed2, nedges, result);
}

void generate_igraph(igraph_t *igraph, int scale, uint64_t seed1, int seed2) {
    igraph_vector_t edges_vector;

    // igraph use 2 item for an edge so we need 2x vector length
    int64_t nedges = (int64_t)(EDGE_FACTOR) << (scale + 1);
    packed_edge* result;

    generate_graph(scale, seed1, seed2, &result);

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

void hist(igraph_t *igraph, vector<int> &frequencies){
    igraph_vector_t v;

    igraph_vector_init(&v, 0);

    igraph_degree(igraph, &v, igraph_vss_all(), IGRAPH_ALL, IGRAPH_NO_LOOPS);

    for (long i=0; i<igraph_vector_size(&v); i++) {
        int degree = (long int) VECTOR(v)[i];

        frequencies[degree] += 1;
    }
}


int main(int argc, char** argv) {
    aml_init(&argc,&argv); //includes MPI_Init inside
    setup_globals();

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist(1,100); // distribution in range [1, 6]

    double tests_start = MPI_Wtime();

    int scale = SCALE;
    int test_amount = TESTS_AMOUNT;
    int seed = dist(rng);

    int diameter_sum = 0;
    double distance_sum = 0;

    int nvert = 1 << scale;
    vector<int> histogram(nvert, 0);
    for (int k = 0; k < test_amount; ++k) {
        uint64_t seed1 = k * seed, seed2 = k + seed;

        igraph_t graph;
        generate_igraph(&graph, scale, seed1, seed2);


        diameter_sum += diameter(&graph);
        distance_sum += avg_distance(&graph);
        hist(&graph, histogram);


        igraph_destroy(&graph);
    }

    double avg_diameter = diameter_sum  / (double)test_amount;
    double avg_distance = distance_sum  / (double)test_amount;

    printf("Avg diameter of the graph: %f\n",
           avg_diameter);

    printf("Avg distance of the graph: %f\n",
           avg_distance);

    ofstream output("output", ios_base::out | ios_base::trunc);


    int block = 100;
    int i = 0;
    int acc = 0;
    for(auto it = histogram.cbegin(); it != histogram.cend(); ++it) {

        if (i < block) {
            acc += *it;
            ++i;
        } else {
            output << acc / (double) TESTS_AMOUNT / nvert << "\n";
            acc = *it;
            i = 1;
        }
    }

    double tests_stop = MPI_Wtime();
    double tests_time = tests_stop - tests_start;

    fprintf(stderr, "running time:               %f s\n", tests_time);


    return 0;
}





