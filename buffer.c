#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define _1Gb         1000000              /* 1 gigabit */
#define THROUGHPUT   21                   /* 21Gbps is the minimal */
#define MAXV         100                  /* maximum number of vertices */
#define MAXDEGREE    50                   /* maximum vertex outdegree */
#define MSS          9000                 /* maximum segment size in bytes */
#define WIN          64                   /* maximum or required buffer size in KBytes */
#define DEFAULT      12                   /* IEEE 802.3bj standard considers acceptable
                                           * a Bit Error Rate (BER) of 1 in 1*10^12 bits */

typedef enum { 
    false,
    true
} bool;

bool processed[MAXV];                     /* which vertices have been processed */
bool discovered[MAXV];                    /* which vertices have been found */
bool finished = false;                    /* found all solutions yet? */
int parent[MAXV];                         /* discovery relation */
double pings[MAXV];                       /* round trip times */
double rating[MAXV];                      /* throuput rates */
int links = 0;                            /* TCP links */

typedef struct {
	int vertex_conn;

	double rtt;
} edge_t;

typedef struct {
	int vid;
	int outdegree;                        /* outdegree of each vertex */

	bool has_buffer;
	double buffer_size;
} vertex_t;

typedef struct {
	edge_t edges[MAXV+1][MAXDEGREE];      /* adjacency info */
	vertex_t vertex[MAXV+1];              /* vertices info */
	int nvertices;                        /* number of vertices in graph */
	int nedges;                           /* number of edges in graph */
	double bit_error;
} graph_t;

static int
initialize_graph(graph_t *graph)
{
	int counter;

	graph->nvertices = 0;
	graph->nedges = 0;

	for (counter = 0; counter < MAXV; counter++) {
		graph->vertex[counter].vid = 0;
		graph->vertex[counter].outdegree = 0;
		graph->vertex[counter].has_buffer = false;
		graph->vertex[counter].buffer_size = 0;
	}

	return 0;
}

static int
insert_edge(graph_t *graph,
	    int vertex,
	    int vertex_conn,
	    double rtt,
	    bool directed)
{
	if (graph->vertex[vertex].outdegree > MAXDEGREE)
		printf("Warning: insertion(%d,%d) exceeds max degree\n", vertex, vertex_conn);

	graph->edges[vertex][graph->vertex[vertex].outdegree].vertex_conn = vertex_conn;
	graph->edges[vertex][graph->vertex[vertex].outdegree].rtt = rtt;
	graph->vertex[vertex].outdegree++;
	graph->vertex[vertex].vid = vertex;

	if (directed == false)
		insert_edge(graph, vertex_conn, vertex, rtt, true);
	else
		graph->nedges += 2;

	return 0;
}

static int
read_graph(graph_t *graph,
	   bool directed)
{
	float exponent;
	int counter;
	int nedges;
	int vertex, vertex_conn;
	double rtt;

	initialize_graph(graph);
	printf("Insert number of degrees and edges:\n");
	scanf("%d %d", &(graph->nvertices), &nedges);

	printf("There're: %d degrees and %d edges, now spell'em with RTT\n",
			graph->nvertices, nedges);
	for (counter = 0; counter < nedges; counter++) {
		scanf("%d %d %lf", &vertex, &vertex_conn, &rtt);
		insert_edge(graph, vertex, vertex_conn, rtt, directed);
	}

	printf("bit error = 1 in 10 ^ (-X)\n");
	scanf("%f", &exponent);

	if(exponent == 0)
		exponent = DEFAULT;

	exponent = exponent/2;
	graph->bit_error = pow(10, exponent);

	return 0;
}

/*
 * Rate <= (MSS/RTT)*(1 / sqrt{error})
 */

static void
get_throuput_rate(graph_t *graph, int i, int j)
{
	for (i = i; i < graph->nedges; i++) {
		for (j = j; j < graph->nvertices; j++) {
            pings[links] += graph->edges[i][j].rtt/2;

            if (graph->vertex[i].has_buffer == true) {
                links++;
                get_throuput_rate(graph, i, j);
            }
        }
    }

    int x;

    for (x = 0; x <= links; x++)
	    rating[links] = (MSS/pings[links])*graph->bit_error;
}

static int
process_edge(int vertex,
	     int next_vertex)
{
	if (parent[vertex] != next_vertex) {
		printf("Cycle from %d to %d:", next_vertex, vertex);
		find_path(next_vertex, vertex, parent);
		finished = true;
	}
}

static int
process_vertex(graph_t *graph, int vertice)
{
    if (graph->vertex[vertice].has_buffer == false)
        graph->vertex[vertice].has_buffer = true;
    else
        graph->vertex[vertice].has_buffer = false;

	return 0;
}

static bool
valid_edge(int vertex_conn)
{
    return true;
}

static int
depth_first_search(graph_t *graph,
                   int vertice)
{
	int counter;
	int next_vertex;
	int vertex = graph->vertex[vertice].vid;
	int outdegree = graph->vertex[vertice].outdegree;

	if (finished)
		return 0;

	discovered[vertex] = true;

	process_vertex(graph, vertex);

	for (counter = 0; counter < outdegree; counter++) {
		next_vertex = graph->edges[vertex][counter].vertex_conn;

		if (valid_edge(graph->edges[vertex][counter].vertex_conn) == true) {

			if (discovered[next_vertex] == false) {
				parent[next_vertex] = vertex;

	            rating[links] = (unsigned int)get_throuput_rate(graph);

	            process_vertex(graph, vertex);    
				depth_first_search(graph, next_vertex);

			} else if (processed[next_vertex] == false)
	            process_edge(vertex, next_vertex);
		}

		if (finished)
			return 0;
	}
	processed[vertex] = true;

	return 0;
}

static int
print_graph(graph_t *graph)
{
	int vertexCont, edgeCont;

	for (vertexCont = 1; vertexCont <= graph->nvertices; vertexCont++) {
		printf("%d: ", vertexCont);

		for (edgeCont = 0; edgeCont < graph->vertex[vertexCont].outdegree; edgeCont++)
			printf(" %d ", graph->edges[vertexCont][edgeCont].vertex_conn);

		printf("\n");
	}

	return 0;
}

int main()
{
	graph_t *graph;
	unsigned int rate;

	graph = malloc(sizeof(graph_t));
	memset(graph, 0, sizeof(graph));
    memset(rating, 0, sizeof(rating));

	read_graph(graph, false);
	print_graph(graph);

	rate = (unsigned int)get_throuput_rate(graph, 0, 0);

	if(rate/_1Gb >= THROUGHPUT)
		printf("\nOK\n");
	else
		depth_first_search(graph, 2);

	free(graph);

	return 0;
}
