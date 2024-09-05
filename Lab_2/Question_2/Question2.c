#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define CACHE_SIZE 5

// Structure to represent a cached web page
// It contains fields for the URL and content of the web page, as well as a pointer to the next cached page.
typedef struct WebPage {
    char url[256];
    char content[4096];
    struct WebPage* next;
} WebPage;

// Global variables
WebPage* cache = NULL;
int cache_count = 0; //to keep track of the number of web pages currently cached

// Function to add a web page to the cache
void add_to_cache(const char* url, const char* content)// responsible for adding a web page to the cache
 {
    // Create a new web page node
    WebPage* new_page = (WebPage*)malloc(sizeof(WebPage));
    strcpy(new_page->url, url);
    strcpy(new_page->content, content);
    new_page->next = cache;

    // Update the cache
    cache = new_page;
    cache_count++;

    // If the cache size exceeds the limit, remove the LRU page
    if (cache_count > CACHE_SIZE) {
        WebPage* prev = NULL;
        WebPage* current = cache;

        // Traverse to the end to find the LRU page
        while (current->next != NULL) {
            prev = current;
            current = current->next;
        }

        // Remove the LRU page
        prev->next = NULL;
        free(current);
        cache_count--;
    }
}

// Function to retrieve a web page from the cache or fetch it using HTTP GET
const char* get_web_page(const char* url)//responsible for displaying the contents of the cache
 {
    // Check if the page is already in the cache
    WebPage* current = cache;
    while (current != NULL) {
        if (strcmp(current->url, url) == 0) {
            // Move the accessed page to the front to mark it as most recently used
            if (current != cache) {
                WebPage* prev = cache;
                while (prev->next != current) {
                    prev = prev->next;
                }
                prev->next = current->next;
                current->next = cache;
                cache = current;
            }
            return current->content; // Return the cached content
        }
        current = current->next;
    }

    // If the page is not in the cache, fetch it using HTTP GET
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        return NULL;
    }

    server = gethostbyname(url);
    if (server == NULL) {
        perror("Error, no such host");
        return NULL;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // AF_INET is a constant representing the address family for IPv4.
    serv_addr.sin_port = htons(80); //  it sets the serv_addr.sin_port field to port 80. Port 80 is commonly used for HTTP 
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error connecting");
        return NULL;
    }

    char request[512];
    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", url);

    n = write(sockfd, request, strlen(request));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    char response[4096];
    memset(response, 0, sizeof(response));
    n = read(sockfd, response, sizeof(response) - 1);
    if (n < 0) {
        perror("Error reading from socket");
        return NULL;
    }

    // Add the fetched web page to the cache
    add_to_cache(url, response);

    // Close the socket
    close(sockfd);

    return cache->content; // Return the newly fetched content
}

// Function to display the contents of the cache
void display_cache() {
    WebPage* current = cache;
    while (current != NULL) {
        printf("URL: %s\n", current->url);
        current = current->next;
    }
}

int main() {
    char url[256];

    while (1) {
        printf("Enter the URL of the web page to fetch (or 'q' to quit): ");
        fgets(url, sizeof(url), stdin);
        url[strcspn(url, "\n")] = '\0'; // Remove the newline character if present

        if (strcmp(url, "q") == 0) {
            break; // Exit the loop if the user inputs 'q'
        }

        const char* content = get_web_page(url);
        if (content != NULL) {
            printf("Content:\n%s\n", content);
        }

        printf("\nCache Contents:\n");
        display_cache();
    }

    return 0;
}

