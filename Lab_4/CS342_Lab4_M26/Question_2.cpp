#include <iostream>
#include <queue>
#include <random>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <bits/stdc++.h>
using namespace std;

const int SimulationTimeInMinutes = 100; // Total simulation time in minutes

// Function to generate random exponential service times
default_random_engine arrival,service;



exponential_distribution<double> arr(4.0),ser(5.0);
vector<double> v;
double sum=0;
double generateArrivalTime(double lambda) {
    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<double> exponential(lambda);
    return exponential(gen);
}

double generateServiceTime(double lambda) {
    random_device rd;
    mt19937 gen(rd());
    exponential_distribution<double> exponential(lambda);
    return exponential(gen);
}

// Function for passenger arrival and queuing
// The function simulates the arrival of passengers, their entry into queues, 
// and the queuing process. The simulation considers random inter-arrival times, 
// assigns passengers to security lines, and prints relevant information. 
// After the simulation time is over, any remaining passengers are moved to the queues.
void passengerArrivalAndQueuing(double lambda, double mu, int K, vector<queue<double>>& q, vector<int>& totalPassengers, vector<mutex>& passengersMutex, mutex& outputMutex) {
    double currentTime = 0.0; // Initialize the simulation time
    int i = 0; // Initialize the passenger count
    int S = q.size(); // Get the number of security scanners

    while (currentTime < SimulationTimeInMinutes) { // Loop until the total simulation time is reached
        int idx = rand() % S; // Randomly select a security line
        if (q[idx].size() >= K) continue; // If the queue is full, skip this iteration

        double interArrivalTime = generateArrivalTime(lambda); // Generate random inter-arrival time
        passengersMutex[idx].lock(); // Acquire lock to update passenger information for this security line
        i++; // Increment passenger count
        currentTime += interArrivalTime; // Update the current time with the inter-arrival time
        q[idx].push(currentTime); // Passenger enters the queue for the selected security line
        totalPassengers[idx]++; // Update the total passengers for this security line
        passengersMutex[idx].unlock(); // Release the lock

        outputMutex.lock(); // Acquire lock for output
        cout << "Passenger " << i << " arrived at time " << fixed << setprecision(2) << currentTime << " at Security Line " << idx << endl;
        outputMutex.unlock(); // Release the lock

        this_thread::sleep_for(chrono::milliseconds(100)); // Simulate processing time
    }

    // Move any remaining passengers to the queues after simulation ends
    for (int i = 0; i < S; i++) {
        passengersMutex[i].lock(); // Acquire lock
        q[i].push(currentTime); // Move passengers to the queue
        passengersMutex[i].unlock(); // Release lock
    }
}


// Function for passenger servicing
// This function simulates the servicing of passengers at a security scanner, 
// considering arrival times, service times, and updating relevant statistics. 
// The loop continues until there are no more passengers to be serviced. The 
// function also ensures proper locking mechanisms for thread safety.
void passengerServicing(int scanner, double mu, vector<queue<double>>& queue, vector<double>& totalTimeInSystem, vector<double>& totalTimeWaitingInQueue, vector<mutex>& passengersMutex, mutex& outputMutex, vector<mutex>& timeMutex, vector<double>& totalServiceTime) {
    double currentTime = 0; // Initialize the current time for the scanner
    int i = 1; // Initialize the passenger count for the scanner

    while (true) { // Infinite loop for servicing passengers
        passengersMutex[scanner].lock(); // Acquire lock for the scanner's queue
        if (!queue[scanner].empty()) { // If the queue is not empty
            double arrivalTime = queue[scanner].front(); // Get the arrival time of the next passenger
            if (arrivalTime > SimulationTimeInMinutes) { // If the arrival time is beyond the simulation time, exit the loop
                passengersMutex[scanner].unlock(); // Release the lock
                break;
            }
            queue[scanner].pop(); // Remove the serviced passenger from the queue
            passengersMutex[scanner].unlock(); // Release the lock

            if (arrivalTime > currentTime) { // Update the current time if needed
                currentTime = arrivalTime;
            }

            double serviceTime = generateServiceTime(mu); // Generate random service time
            currentTime += serviceTime; // Update the current time with the service time
            timeMutex[scanner].lock(); // Acquire lock for updating time-related statistics
            totalTimeWaitingInQueue[scanner] += (currentTime - arrivalTime - serviceTime); // Update total waiting time
            totalTimeInSystem[scanner] += currentTime - arrivalTime; // Update total system time
            totalServiceTime[scanner] += serviceTime; // Update total service time
            timeMutex[scanner].unlock(); // Release the lock

            outputMutex.lock(); // Acquire lock for output
            cout << "Passenger " << i << " serviced at time " << fixed << setprecision(2) << currentTime << " at Security Line " << scanner << endl; // Print servicing information
            outputMutex.unlock(); // Release the lock
            i++; // Increment passenger count
        } else {
            passengersMutex[scanner].unlock(); // Release the lock if the queue is empty
        }
    }
}


int main() {
    double lambda; // Arrival rate (passengers per unit of time)
    double mu;     // Service rate (passengers processed per unit of time)
    int K;         // Buffer size
    int S;
    cout << "Enter arrival rate (lambda): ";
    cin >> lambda;
    cout << "Enter service rate (mu): ";
    cin >> mu;
    cout << "Enter buffer size (K): ";
    cin >> K;
    cout << "Enter number of scanners: ";
    cin >> S;
    cout << "Hello" ;

    arrival.seed(time(NULL));
    service.seed(time(NULL));

    arr = exponential_distribution<double>(lambda);
    ser = exponential_distribution<double>(mu);


    vector<queue<double>> q(S); // vector of queues to hold passengers 
    vector<double> totalTimeInSystem(S, 0.0);
    vector<double> totalTimeWaitingInQueue(S, 0.0);
    vector<double> totalServiceTime(S, 0.0);
    vector<int>  totalPassengers(S,0);
    vector<mutex> passengersMutex(S); // Mutex for totalPassengers
    mutex outputMutex; // Mutex for thread-safe output
    vector<mutex> timeMutex(S); // Mutex for thread-safe output
    
    
    // // Threads for passenger arrival/queuing and servicing
    thread arrivalThread(passengerArrivalAndQueuing, lambda, mu, K, ref(q),  ref(totalPassengers), ref(passengersMutex), ref(outputMutex));
    vector<thread> temp;
    for(int i = 0; i < S; i++){
        temp.push_back(thread (passengerServicing, i, mu, ref(q), ref(totalTimeInSystem),ref(totalTimeWaitingInQueue),ref(passengersMutex), ref(outputMutex),ref(timeMutex),ref(totalServiceTime)));
    }


    arrivalThread.join();
    for(thread& thread: temp){
        thread.join();
    }
    // Calculate and report statistics with output mutex protection

    for(int i = 0; i < S; i++)  
    {   
        cout << endl;
        cout<< "SCANNER " << i << endl;
        lock_guard<mutex> lock1(outputMutex);
        lock_guard<mutex> lock2(timeMutex[i]);
        cout << "total passengers: " << totalPassengers[i] << endl;
        cout << "Average System Time: " << totalTimeInSystem[i] / (totalPassengers[i]-1) << " minutes" << endl;
        cout << "Average Waiting Time: " << totalTimeWaitingInQueue[i] / (totalPassengers[i]-1) << " minutes" << endl;
        cout << "Average Queue Length: " << totalTimeInSystem[i] / SimulationTimeInMinutes << " passengers/minute" << endl;
        cout << "System Utilization: " << (totalServiceTime[i]/ SimulationTimeInMinutes) * 100 << "%" << endl;
    }

    return 0;
}