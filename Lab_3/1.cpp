#include <bits/stdc++.h> // Include the C++ Standard Library and necessary headers.
using namespace std;

#define MAX_ROUTERS 10
#define no_parent = -1; // Define constants.

class router
{
public:
	int router_id; // Router's unique ID.

	vector<pair<router *, int>> neighbours; // Store neighbor routers and their edge weights.

	unordered_map<int, int> routing_table; // Routing table for the router.

	// Method to add a neighbor router with an associated weight.
	void add_neighbour(router *new_neighbour, int weight)
	{
		neighbours.push_back(make_pair(new_neighbour, weight));
	}

	// Data structures for Dijkstra's algorithm:
	vector<int> dist, parent;
	vector<int> nxt_hop;

	// Method to update the routing table using Dijkstra's algorithm.
	void update_routing_table(router *src, int V)
	{
		// Initialize distance and parent vectors for all routers.
		for (int i = 0; i < V; i++)
		{
			dist.push_back(INT_MAX);
			parent.push_back(-1);
		}

		// Priority queue for the shortest path algorithm.
		priority_queue<pair<int, router *>, vector<pair<int, router *>>, greater<pair<int, router *>>> pq;

		// Set the distance of the source router to 0.
		pq.push(make_pair(0, src));
		dist[src->router_id] = 0;

		while (!pq.empty())
		{
			// Get the router with the shortest distance.
			router *u = pq.top().second;
			pq.pop();

			// Iterate through its neighbors.
			for (auto i = u->neighbours.begin(); i != u->neighbours.end(); ++i)
			{
				router *v = (*i).first;
				int weight = (*i).second;

				// Relaxation: Update distance and parent if a shorter path is found.
				if (dist[v->router_id] > dist[u->router_id] + weight)
				{
					dist[v->router_id] = dist[u->router_id] + weight;
					parent[v->router_id] = u->router_id;
					pq.push(make_pair(dist[v->router_id], v));
				}
			}
		}
	}

	// Helper method to recursively print the parents in the path.
	void print_parents(int a)
	{
		if (a == -1)
			return;
		cout << " -- " << a << ' ';
		print_parents(parent[a]);
	}

	// Method to print the routing table for the router.
	void print_routing_table(router *src, int n)
	{
		cout << "Routing table of " << src->router_id << endl;
		for (int i = 0; i < n; ++i)
		{
			cout << "R" << i << "	" << dist[i] << "		";
			cout << i << ' ';
			print_parents(parent[i]);
			cout << endl;
		}
	}
};

int main()
{
	int n;
	cout << "Enter the number of routers" << endl;
	cin >> n;
	router *arr[n]; // Create an array of router objects.
	for (int i = 0; i < n; i++)
	{
		arr[i] = new router; // Initialize router objects with unique IDs.
		arr[i]->router_id = i;
	}
	int k;
	cout << "Enter the number of edges" << endl;
	cin >> k;

	cout << "Enter the two routers and the weight between them" << endl;

	for (int i = 0; i < k; i++)
	{
		int x, y, w;
		cin >> x >> y >> w;
		arr[x]->add_neighbour(arr[y], w); // Add neighbors and weights.
		arr[y]->add_neighbour(arr[x], w); // Create undirected connections.
	}

	// Calculate and print routing tables for each router.
	for (int i = 0; i < n; i++)
	{
		arr[i]->update_routing_table(arr[i], n);
		arr[i]->print_routing_table(arr[i], n);
		cout << endl;
	}

	return 0;
}
