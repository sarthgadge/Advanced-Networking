
// The provided code is a C++ program for a simple DNS lookup 
// tool that converts domain names into their corresponding IP addresses.


// These are the include statements to include necessary libraries 
// for socket programming, string manipulation, and other standard functions.
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

// These are preprocessor directives defining constants for the buffer size, 
// DNS server IP address (Google's public DNS server), port number, and 
// width used for formatting hex dumps.
#define BUF_SIZE 512
#define SERVER "8.8.8.8"
#define PORT 53
#define WIDTH 16

//  a structure named query is defined to hold information related 
// to the DNS query. It includes the length of the query, the URL being
// queried, the request buffer, and the request type.
struct query
{
    uint16_t length;
    string url;
    unsigned char request[BUF_SIZE];
    uint16_t reqType;
};

// an instance of the query structure named dnsQuery is initialized with 
// some default values. These values represent a basic DNS query packet.
struct query dnsQuery = {
    .length = 12,
    .url = "",
    .request = {
        0xDB,
        0x42,
        0x01,
        0x00,
        0x00,
        0x01,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    },
    .reqType = 0x01
};

void intializeRequest(char * url) // parses the URL and constructs the DNS query
{
    char * word;
    int i;
    printf("Asking DNS server %s about %s\n", SERVER, url);
    dnsQuery.url = strdup(url);
    dnsQuery.reqType = 0x01;

    word = strtok(url, ".");
    while (word)
    {
        printf("Parsing hostname: \"%s\" is %ld characters\n", word, strlen(word));
        dnsQuery.request[dnsQuery.length++] = strlen(word);
        for (i = 0; i < strlen(word); i++)
        {
            dnsQuery.request[dnsQuery.length++] = word[i];
        }
        word = strtok(NULL, ".");
    }

    dnsQuery.request[dnsQuery.length++] = 0x00; 
    dnsQuery.request[dnsQuery.length++] = 0x00; 
    dnsQuery.request[dnsQuery.length++] = dnsQuery.reqType;
    dnsQuery.request[dnsQuery.length++] = 0x00; 
    dnsQuery.request[dnsQuery.length++] = 0x01;
}

void hexdump(string desc, unsigned char * addr, int len) 
// takes the data (address) and its length (len) as input 
// and prints it in hexadecimal format.
// This is used to display the DNS query and response packets.
{
    int i;
    unsigned char buff[17];
    unsigned char * pc = addr;

    if (desc != "") cout << desc << ":" << endl;

    for (i = 0; i < len; i++) {

        if ((i % 16) == 0)
        {
            if (i != 0)
                printf("  %s\n", buff);

            printf("  %04x ", i);
        }

        printf(" %02x", pc[i]);

        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) buff[i % 16] = '.';
        else buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    while ((i % 16) != 0)
    {
        printf("   ");
        i++;
    }

    printf("  %s\n", buff);
}

void search(map < string, vector < string > > & cache) 
// responsible for sending the DNS query to the DNS server
// and processing the response. It also updates a cache with 
// the DNS responses to improve lookup speed for frequently 
// visited domain names
{
    struct sockaddr_in addr;// used to store network address information for the DNS server.
    int sd; // socket descriptor used to manage the socket for communication with the DNS server.
    int ret, rcode, i;// store various response-related values.
    int ip = 0, dom = 0;// count the number of IP addresses and domain names
    int length;// length of the DNS response
    unsigned char buffer[BUF_SIZE];//  store the DNS response data received from the server
    unsigned char tempBuf[3];// temporary storage of data during processing
    uint16_t QDCOUNT; 
    uint16_t ANCOUNT; 
    uint16_t NSCOUNT; 
    uint16_t ARCOUNT; 
    uint16_t QCLASS; 
    uint16_t ATYPE; 
    uint16_t ACLASS; 
    uint32_t TTL; 
    uint16_t RDLENGTH; 
    uint16_t MSGID;
    //  Creates a UDP socket (address family IPv4,0 is the udp protocol)
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    //checking socket creation
    if(sd < 0)
    {
        cout<<"Error in creating the socket"<<'\n';
        exit(1);
    }


    memset( & addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(SERVER);
    addr.sin_port = htons(PORT);
    unsigned int size = sizeof(addr);

    hexdump("Sending packet:", dnsQuery.request, dnsQuery.length);
    printf("Length of the packet:%d\n", dnsQuery.length);

    if(sendto(sd, dnsQuery.request, dnsQuery.length, 0, (struct sockaddr*) & addr, size) < 0)
    {
        printf("Error Sending Request");
        exit(1);
    }

    memset( & buffer, 0, BUF_SIZE);

    if((ret = recvfrom(sd, buffer, BUF_SIZE, 0, (struct sockaddr * ) & addr, & size)) < 0)
    {
        printf("Error Receiving Response");
        exit(1);
    }

    hexdump("Received packet:", buffer, ret);
    close(sd);

    rcode = (buffer[3] & 0x0F);

    QDCOUNT = (uint16_t) buffer[4] * 0x100 + buffer[5];
    printf("Entries in question section: %u\n", QDCOUNT);

    ANCOUNT = (uint16_t) buffer[6] * 0x100 + buffer[7];
    printf("Records in answer section: %u\n", ANCOUNT);

    NSCOUNT = (uint16_t) buffer[8] * 0x100 + buffer[9];
    printf("Name server resource record count: %u\n", NSCOUNT);

    ARCOUNT = (uint16_t) buffer[10] * 0x100 + buffer[11];
    printf("Additional records count: %u\n", ARCOUNT);

    printf("Query type: %u\n", dnsQuery.reqType);

    QCLASS = (uint16_t) dnsQuery.request[dnsQuery.length - 2] * 0x100 + dnsQuery.request[dnsQuery.length - 1];
    printf("Query class: %u\n", QCLASS);

    length = dnsQuery.length + 1;

    ATYPE = (uint16_t) buffer[length + 1] * 0x100 + buffer[length + 2];
    printf("Answer type: %u\n", ATYPE);

    ACLASS = (uint16_t) buffer[length + 3] * 0x100 + buffer[length + 4];
    printf("Answer class: %u\n", ACLASS);

    TTL = (uint32_t) buffer[length + 5] * 0x1000000 + buffer[length + 6] * 0x10000 + buffer[length + 7] * 0x100 + buffer[length + 8];
    printf("Seconds to cache: %u\n", TTL);

    RDLENGTH = (uint16_t) buffer[length + 9] * 0x100 + buffer[length + 10];
    printf("Bytes in answer: %u\n", RDLENGTH);

    MSGID = (uint16_t) buffer[0] * 0x100 + buffer[1];
    printf("Answer message id: %u\n", MSGID);

    if (rcode == 2)
    {
        printf("Nameserver %s returned SERVFAIL:\n", SERVER);
        printf("The name server was unable to process this query due to a problem with the name server.\n");
        exit(1);
    }
    else if (rcode == 3)
    {
        cout << "nameserver " << SERVER << " returned NXDOMAIN for " << dnsQuery.url << ":\n";
        printf("The domain name referenced in the query does not exist\n");
        exit(1);
    }

    vector < string > temp;
    string strauxil = "";
    if (dnsQuery.reqType == 0x01)
    {
        printf("DNS server's answer is: (type#=%u):", ATYPE);
        for (i = 0; i < ret; i++)
        {
            if (buffer[i] == 0xC0 && buffer[i + 3] == 0x01) {
                ip++;
                i += 12;
                printf(" %u.%u.%u.%u\n", buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3]);
                strauxil += to_string(buffer[i]) + "." + to_string(buffer[i + 1]) + "." + to_string(buffer[i + 2]) + "." + to_string(buffer[i + 3]);
                temp.push_back(strauxil);
                strauxil = "";
            }
            cache[dnsQuery.url] = temp;

        }

        if (!ip)
        {
            printf("No IPv4 address found in the DNS response!\n");
            exit(1);
        }
    }

}

int main() 
{
    // It includes a loop that repeatedly takes input from the user (domain names), 
    // checks if the domain name is already in the cache, and either fetches it from 
    // the cache or sends a DNS query to the server. The loop continues until the 
    // user enters "exit" to quit the program

    string myurl;
    char url[100];
    map <string,vector<string> > cache;
    while (1)
    {
        cout<<"***********************************NEW REQUEST*********************************"<<'\n';
        cout << "Please enter an url..or else enter exit to quit.." << endl;
        cin >> myurl;
        strcpy(url,myurl.c_str());

        if (strcmp(url, "exit") == 0)
        {
            cout << "Exited.." << endl;
            exit(1);
        }

        if (cache.find(url) == cache.end())
        {
            dnsQuery.length = 12;
            dnsQuery.url = "";
            dnsQuery.request[0] = 0xDB;
            dnsQuery.request[1] = 0x42;
            dnsQuery.request[2] = 0x01;
            dnsQuery.request[3] = 0x00;
            dnsQuery.request[4] = 0x00;
            dnsQuery.request[5] = 0x01;
            dnsQuery.request[6] = 0x00;
            dnsQuery.request[7] = 0x00;
            dnsQuery.request[8] = 0x00;
            dnsQuery.request[9] = 0x00;
            dnsQuery.request[10] = 0x00;
            dnsQuery.request[11] = 0x00;
            dnsQuery.reqType = 0x01;
            cout << "Not found in cache.. going to server" << endl;
            intializeRequest(url);
            search(cache);
        }
        else
        {
            cout << "Found in cache ..." << endl;
            cout << cache[url].size() << endl;

            for (int i = 0; i < cache[url].size(); i++)
            {
                cout << cache[url][i];
                cout << endl;
            }
        }
    }
}