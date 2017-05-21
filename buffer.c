#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXV         100                  /* maximum number of vertices */
#define MAXDEGREE    50                   /* maximum vertex outdegree */
#define MSS          9000                 /* maximum segment size in bytes */
#define WIN          64                   /* maximum or required buffer size in KBytes */
#define C            sqrt(3/2)            /* constant */

typedef enum { false, true } bool;

bool processed[MAXV];                     /* which vertices have been processed */
bool discovered[MAXV];                    /* which vertices have been found */
bool finished = false;                    /* found all solutions yet? */
int parent[MAXV];                         /* discovery relation */

typedef struct {
	int vertex_conn;

	unsigned int rtt;
	unsigned int bit_error;
} edge_t;

typedef struct {
	int vid;
	int outdegree;                    /* outdegree of each vertex */

	bool has_buffer;
	unsigned int buffer_size;
} vertex_t;

typedef struct {
	edge_t edges[MAXV+1][MAXDEGREE];  /* adjacency info */
	vertex_t vertex[MAXV+1];          /* vertices info */
	int nvertices;                    /* number of vertices in graph */
	int nedges;                       /* number of edges in graph */
	unsigned int bit_error;
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
	    unsigned int rtt,
	    unsigned int bit_error,
	    bool directed)
{
	if (graph->vertex[vertex].outdegree > MAXDEGREE)
		printf("Warning: insertion(%d,%d) exceeds max degree\n", vertex, vertex_conn);

	graph->edges[vertex][graph->vertex[vertex].outdegree].vertex_conn = vertex_conn;
	graph->edges[vertex][graph->vertex[vertex].outdegree].bit_error = bit_error;
	graph->edges[vertex][graph->vertex[vertex].outdegree].rtt = rtt;
	graph->vertex[vertex].outdegree++;

	/* link is bidirectional, so we duplicate... */
	graph->edges[vertex_conn][graph->vertex[vertex_conn].outdegree] =
					graph->edges[vertex][graph->vertex[vertex].outdegree];
	graph->vertex[vertex_conn].outdegree++;

	if (directed == false)
		insert_edge(graph, vertex, vertex_conn, rtt, bit_error, true);
	else
		graph->nedges += 2;

	return 0;
}

static int
read_graph(graph_t *graph,
	   bool directed)
{
	int counter;
	int nedges;
	int vertex, vertex_conn, rtt, bit_error;

	initialize_graph(graph);
	printf("Insert number of degress and edges:\n");
	scanf("%d %d", &(graph->nvertices), &nedges);

	printf("There're: %d nodes and %d edges, now spell'em with RTT and Bit error\n",
			graph->nvertices, nedges);
	for (counter = 0; counter < nedges; counter++) {
		scanf("%d %d %d %d", &vertex, &vertex_conn, &rtt, &bit_error);
		insert_edge(graph, vertex, vertex_conn, rtt, bit_error, directed);
	}

	return 0;
}

static unsigned int
get_total_rtt(graph_t *graph)
{
    int i, j;
    unsigned int rtt = 0;
    
    for (i = 0; i < graph->nvertices; i++)
        for (j = 0; j < graph->nedges; j++)
            rtt += graph->edge[i][j].rtt;
        
    return rtt;
}

static int
get_total_throuput(graph_t *graph)
{
    return 0;
}

static int
process_edge(int vertex,
             int next_vertex)
{
	if (parent[vertex] != next_vertex) {             /* found back edge! */
		printf("Cycle from %d to %d:", next_vertex, vertex);
		find_path(next_vertex, vertex, parent);
		finished = true;
	}
}

static int
process_vertex(int vertex)
{
    return 0;
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
		return;

	discovered[vertex] = true;

	process_vertex(vertex);

	for (counter = 0; counter < outdegree; counter++) {
		next_vertex = graph->edges[vertex][counter].vertex_conn;

		if (valid_edge(graph->edges[vertex][counter].vertex_conn) == true) {

			if (discovered[next_vertex] == false) {
				parent[next_vertex] = vertex;
				depth_first_search(graph, next_vertex);

			} else if (processed[next_vertex] == false)
				process_edge(vertex, next_vertex);
		}

		if (finished)
			return;
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
			printf(" %d (rrt = %u err = %u)",
					graph->edges[vertexCont][edgeCont].vertex_conn,
                                        graph->edges[vertexCont][edgeCont].rtt,
					graph->edges[vertexCont][edgeCont].bit_error);

		printf("\n");
	}

	return 0;
}

int main()
{
	graph_t *graph;

	graph = malloc(sizeof(graph_t));
	memset(graph, 0, sizeof(graph));

	read_graph(graph, true);
	print_graph(graph);

	free(graph);

	return 0;
}
