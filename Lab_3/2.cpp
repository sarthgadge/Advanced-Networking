#include <bits/stdc++.h> // Include the C++ Standard Library and other libraries to use data structures and functions.
using namespace std; // Use the standard C++ namespace for convenience.

class Node{
    public:
        int id;
        bool isTransmiting;
        int no_transmits;
        int no_backoffs;
        void Transmit(){
            isTransmiting=false; // Method for a Node to stop transmitting.
        }
};

class Simulator{
    private:
        vector<Node> node; // A vector to store Node objects.
        int N; // Number of Nodes.
        int c=0; // A counter for tracking transmitting Nodes.
        int backoff_interval; // Backoff interval duration.
        float probability; // Transmission probability.
        map<int, vector<int> > mp; // A map for scheduling retransmissions.
        vector<string> log; // A vector to store simulation log.
    public:
        Simulator(int N,int Time, float p,int bi){
            this->N=N; // Initialize the number of Nodes.
            this->probability=p*100; // Initialize the transmission probability as a percentage.
            this->backoff_interval=bi; // Initialize the backoff interval.
            node.resize(N); // Resize the 'node' vector to accommodate 'N' Node objects.
            for(int i=0;i<N;i++){
                node[i].id=i+1; // Assign unique IDs to Nodes.
                node[i].isTransmiting=false; // Initialize Node transmission state.
            }
            simulate(Time); // Start the simulation with a given time.
            Log(); // Print the simulation log.
        }

        void randomFire(int Time){
            // A method to simulate random Nodes initiating transmissions at a given time.
            for(int i=0;i<mp[Time].size();i++){
                node[mp[Time][i]].isTransmiting=true; // Set Nodes to transmitting state.
                c++; // Increase the transmitting Node counter.
            }
            for(int i=0;i<N;i++){
                if(!node[i].isTransmiting){
                    if(rand()%100<probability){
                        node[i].isTransmiting=true; // Set Nodes to transmitting state based on probability.
                        c++; // Increase the transmitting Node counter.
                    }
                }
            }
        }

        void CollisionDetection(int Time){
            // A method to detect and handle collisions.
            if(c>1){ // If there's more than one transmitting Node.
                int i=0;
                for(i=0;i<N;i++){
                    if(node[i].isTransmiting){
                        i++;
                        break;
                    }
                }
                for(i;i<N;i++){
                    if(node[i].isTransmiting){
                        int backoff = rand()%backoff_interval+1; // Generate a random backoff time.
                        mp[Time+backoff].push_back(i); // Schedule retransmissions after the backoff period.
                        c--; // Decrease the transmitting Node counter.
                        node[i].no_backoffs++; // Track the number of backoffs for the Node.
                    }
                }
                log.push_back("Collision"); // Log a collision event.
            }
        }

        void CollisionAvoidance(){
            // A method to handle successful transmissions and empty channels.
            if(c==1){
                for(int i=0;i<N;i++){
                    if(node[i].isTransmiting){
                        node[i].Transmit(); // A successful transmission, so stop transmitting.
                        node[i].no_transmits++; // Track the number of successful transmissions for the Node.
                    }
                }
                c--; // Decrease the transmitting Node counter.
                log.push_back("Success"); // Log a successful transmission event.
            }
            else if(c==0){
                log.push_back("Empty"); // Log an empty channel event.
            }
        }

        void simulate(int Time){
            // A method to run the simulation for a specified duration.
            for(int i=0;i<Time;i++){
                randomFire(i); // Simulate random transmissions.
                CollisionDetection(i); // Detect collisions.
                CollisionAvoidance(); // Handle successful transmissions and empty channels.
            }
        }

        void Log(){
            // A method to print the simulation log and Node statistics.
            for(int i=0;i<log.size();i++){
                cout<<log[i]<<", "; // Print simulation events.
            }
            cout<<endl;
            for(int i=0;i<N;i++){
                printf("Node-%d transmitted %d times, had to backoff %d times\n", node[i].id, node[i].no_transmits, node[i].no_backoffs); // Print Node statistics.
            }
        }

        ~Simulator(){
            // Destructor to clear memory and resources.
            node.clear();
            mp.clear();
            log.clear();
        }
};

int main(){
    // Entry point of the program.
    cout << "\n----------------------------------\n\n";
    Simulator Simulator1(5, 100, 0.01, 5); cout << "\n";     // Less Devices, Less Traffic
    Simulator Simulator2(5, 100, 0.10, 5); cout << "\n";    // Less Devices, More Traffic
    Simulator Simulator3(10, 100, 0.01, 5); cout << "\n";  // More Devices, Less Traffic
    Simulator Simulator4(10, 100, 0.1, 5); cout << "\n";  // More Devices, More Traffic
    cout << "----------------------------------\n\n";
    return 0;
}
