#include <bits/stdc++.h>

using namespace std;

// Class representing an HTTP request
class HttpRequest
{
public:
  int uniqueID;
  int websiteID;
  int processingTime;

  // Constructor to initialize request attributes
  HttpRequest(int id, int webID, int time) : uniqueID(id), websiteID(webID), processingTime(time) {}
};

// Class representing a Website
class Website
{
public:
  int websiteID;
  int ownerID;
  int allocatedBandwidth;
  int allocatedProcessingPower;
  queue<HttpRequest> requestQueue; // Queue to store HTTP requests

  // Default constructor
  Website() = default;

  // Parameterized constructor to initialize website attributes
  Website(int webID, int owner, int bandwidth, int power) : websiteID(webID), ownerID(owner), allocatedBandwidth(bandwidth), allocatedProcessingPower(power) {}
};

// Class representing a Load Balancer
class LoadBalancer
{
public:
  unordered_map<int, Website> websites;
  unordered_map<int, int> resources;
  unordered_map<int, double> weights;
  unordered_map<int, double> finish_times;
  set<int> websiteIDs;

  set<int> usedRequestIDs; // To track used request IDs

  // Method to update weights of websites based on allocated resources
  void updateWeights()
  {
    if (websites.size() == 0)
      return;

    int totalAllocatedResources = 0;

    // Calculate total allocated resources
    for (auto id : websiteIDs)
    {
      totalAllocatedResources += resources[id];
    }

    double min_weight = DBL_MAX;

    // Update weights based on allocated resources
    for (auto id : websiteIDs)
    {
      weights[id] = resources[id] * 1.0 / totalAllocatedResources * 1.0;

      // Track minimum weight
      if (weights[id] < min_weight)
        min_weight = weights[id];
    }

    // Normalize weights
    for (auto id : websiteIDs)
    {
      weights[id] = weights[id] / min_weight;
    }
  }

  // Method to add a new website to the load balancer
  void add_website(int websiteID, int ownerID, int allocatedBandwidth, int allocatedProcessingPower)
  {
    // Check if website ID is already in use
    if (websites.find(websiteID) != websites.end())
    {
      cout << "Website id already in use." << endl;
      return;
    }

    // Add the new website
    websites.emplace(websiteID, Website(websiteID, ownerID, allocatedBandwidth, allocatedProcessingPower));
    websiteIDs.insert(websiteID);
    resources.emplace(websiteID, allocatedBandwidth * allocatedProcessingPower);
    finish_times.emplace(websiteID, 0.0);

    // Update weights after adding a new website
    updateWeights();
  }

  // Method to enqueue an HTTP request
  void enqueue_request(HttpRequest httpRequest)
  {
    // Check if the request ID is already taken
    if (usedRequestIDs.find(httpRequest.uniqueID) != usedRequestIDs.end())
    {
      cout << "Request ID " << httpRequest.uniqueID << " is already taken. Please choose a different one." << endl;
      return;
    }

    // Check if the website ID is valid
    if (websiteIDs.find(httpRequest.websiteID) == websiteIDs.end())
    {
      cout << "Invalid website ID." << endl;
      return;
    }

    // Enqueue the request and mark its ID as used
    websites[httpRequest.websiteID].requestQueue.push(httpRequest);
    usedRequestIDs.insert(httpRequest.uniqueID);
  }

  // Method to dequeue the next HTTP request based on WFQ algorithm
  void dequeue_request()
  {
    double finish_time = DBL_MAX;
    int wfq_wid = -1;

    // Find the website with the next request to be dequeued based on WFQ algorithm
    for (auto id : websiteIDs)
    {
      if (!websites[id].requestQueue.empty())
      {
        double request_finish_time = finish_times[id] + websites[id].requestQueue.front().processingTime * 1.0 / weights[id];

        // Update finish time and website ID if this request finishes earlier
        if (finish_time > request_finish_time)
        {
          finish_time = request_finish_time;
          wfq_wid = id;
        }
        // Handle tie-breaker when finish times are equal
        else if (finish_time == request_finish_time)
        {
          if (weights[id] > weights[wfq_wid])
            wfq_wid = id;
        }
      }
    }

    // Check if there are no requests to dequeue
    if (wfq_wid == -1)
    {
      cout << "No requests to dequeue." << endl;
      return;
    }

    // Dequeue the next request and update finish time
    cout << "Request ID " << websites[wfq_wid].requestQueue.front().uniqueID << " dequeued from website " << websites[wfq_wid].requestQueue.front().websiteID << endl;
    finish_times[wfq_wid] = finish_time;
    websites[wfq_wid].requestQueue.pop();
  }
};

// Function to print HTTP requests in a website's queue
void printHttpRequests(const Website &website)
{
  queue<HttpRequest> tempQueue = website.requestQueue;

  while (!tempQueue.empty())
  {
    const HttpRequest &request = tempQueue.front();
    cout << "ID: " << request.uniqueID << " Time: " << request.processingTime << " | ";
    tempQueue.pop();
  }
}

// Main function
int main()
{
  LoadBalancer loadBalancer;

  // Main loop for user interaction
  while (true)
  {
    // Display website information, including weights and corresponding HTTP request IDs and processing times
    if (loadBalancer.weights.size() != 0)
    {
      cout << "-------------------------------------------------------------------------------------------------" << endl;
      for (auto it : loadBalancer.websiteIDs)
      {
        cout << "Website " << it << " (Weight: " << loadBalancer.weights[it] << ") | ";
        printHttpRequests(loadBalancer.websites[it]);
        cout << endl;
      }
      cout << "-------------------------------------------------------------------------------------------------" << endl;
    }

    // Display menu options
    cout << "Menu:" << endl;
    cout << "1. Add Website" << endl;
    cout << "2. Enqueue HTTP Request" << endl;
    cout << "3. Dequeue HTTP Request" << endl;
    cout << "4. Exit" << endl;

    int choice;
    cout << "Enter your choice: ";
    cin >> choice;

    // Switch-case to handle user choices
    switch (choice)
    {
    case 1:
    {
      int websiteID, ownerID, allocatedBandwidth, allocatedProcessingPower;
      cout << "Enter Website ID, Owner ID, Allocated Bandwidth, and Allocated Processing Power: ";
      cin >> websiteID >> ownerID >> allocatedBandwidth >> allocatedProcessingPower;
      loadBalancer.add_website(websiteID, ownerID, allocatedBandwidth, allocatedProcessingPower);
      break;
    }
    case 2:
    {
      int uniqueID, websiteID, processingTime;
      cout << "Enter Unique ID, Website ID, and Processing Time: ";
      cin >> uniqueID >> websiteID >> processingTime;
      loadBalancer.enqueue_request(HttpRequest(uniqueID, websiteID, processingTime));
      break;
    }
    case 3:
      loadBalancer.dequeue_request();
      break;
    case 4:
      return 0;
    default:
      cout << "Invalid choice. Please try again." << endl;
    }
  }

  return 0;
}