#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define PORT 4455
#define BUFSIZE 1024

// Function prototypes
void display_catalog(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size);
void search_book(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size);
void order_book(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size);
void pay_for_book(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size);
void handle_client(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size);

// Structure for book details
typedef struct book {
    int Serial_Number;
    char Title[50];
    char Authors[50];
    int ISBN;
    char Publisher[50];
    char Date_of_Publication[50];
} book;

// Structure for order details
typedef struct orders {
    int order_number;
    int ISBN;
    char status[50];
} orders;

// Main function for the server application
int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size;
    
    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");  // Listen on all interfaces

    // Bind socket to server address
    bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    
    addr_size = sizeof(clientAddr);
    printf("[+]Listening...\n");

    // Server main loop to handle client requests
    while (1) {
        handle_client(sockfd, &clientAddr, addr_size);
    }
    return 0;
}

// Function to handle client requests
void handle_client(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size) {
    printf("[+]Client connected\n");
    int choice;
    
    // Receive client choice
    recvfrom(serverSocket, &choice, sizeof(int), 0, (struct sockaddr *)clientAddr, &addr_size);
    printf("Data Received: %d\n", choice);

    // Switch based on client choice
    switch(choice) {
        case 1:
            display_catalog(serverSocket, clientAddr, addr_size);
            break;
        case 2:
            search_book(serverSocket, clientAddr, addr_size);
            break;
        case 3:
            order_book(serverSocket, clientAddr, addr_size);
            break;
        case 4:
            pay_for_book(serverSocket, clientAddr, addr_size);
            break;
        case 5:
            exit(EXIT_SUCCESS);  // Exit the program
            break;
        default:
            printf("Invalid choice\n");
            break;
    }
}

// Function to display catalog of books
void display_catalog(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size) {
    FILE* file;
    book books[100];
    int count = 0, book_count;
    
    // Open book file
    file = fopen("bookfile.txt", "r");
    if(file == NULL) {
        printf("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    // Read books from file into array
    while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }

    // Send number of books to client
    sendto(serverSocket, &count, sizeof(int), 0, (struct sockaddr *)clientAddr, addr_size);
    recvfrom(serverSocket, &book_count, sizeof(int), 0, (struct sockaddr *)clientAddr, &addr_size);

    // Send each book to client
    for (int i = 0; i < book_count; i++) {
        sendto(serverSocket, &books[i], sizeof(book), 0, (struct sockaddr *)clientAddr, addr_size);
    }

    // Close file
    fclose(file);
}

// Function to search for a book by title or ISBN
void search_book(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size) {
    FILE* file;
    book books[100];
    int count = 0, choice;
    char status[50];

    // Open book file
    file = fopen("bookfile.txt", "r");
    if(file == NULL) {
        printf("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    // Read books from file into array
    while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }

    // Receive client search choice
    recvfrom(serverSocket, &choice, sizeof(int), 0, (struct sockaddr *)clientAddr, &addr_size);

    // Switch based on search choice
    switch (choice) {
        case 1:
            {
                char title[50];
                recvfrom(serverSocket, title, 50, 0, (struct sockaddr *)clientAddr, &addr_size);
                for (int i = 0; i < count; i++) {
                    if (strcmp(title, books[i].Title) == 0) {
                        strcpy(status, "found");
                        sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);
                        sendto(serverSocket, &books[i], sizeof(book), 0, (struct sockaddr *)clientAddr, addr_size);
                        fclose(file);
                        return;
                    }
                }
                strcpy(status, "not found");
                sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);
            }
            break;
        case 2:
            {
                int ISBN;
                recvfrom(serverSocket, &ISBN, sizeof(int), 0, (struct sockaddr *)clientAddr, &addr_size);
                for (int i = 0; i < count; i++) {
                    if (ISBN == books[i].ISBN) {
                        strcpy(status, "found");
                        sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);
                        sendto(serverSocket, &books[i], sizeof(book), 0, (struct sockaddr *)clientAddr, addr_size);
                        fclose(file);
                        return;
                    }
                }
                strcpy(status, "not found");
                sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);
            }
            break;
    }

    // Close file
    fclose(file);
}

// Function to order a book
void order_book(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size) {
    srand(time(NULL));
    FILE* book_file;
    FILE* order_file;
    book books[100];
    int count = 0, order_number, ISBN;
    char status[50];

    // Open book file
    book_file = fopen("bookfile.txt", "r");
    order_file = fopen("orderfile.txt", "a");
    if(book_file == NULL) {
        printf("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    // Receive ISBN from client
    recvfrom(serverSocket, &ISBN, sizeof(int), 0, (struct sockaddr *)clientAddr, &addr_size);

    // Read books from file into array
    while (fscanf(book_file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }

    // Find book with matching ISBN and create order
    for (int i = 0; i < count; i++) {
        if (ISBN == books[i].ISBN) {
            strcpy(status, "found");
            sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);
            order_number = rand() % 1000;
            fprintf(order_file, "%d\t%d\t%s\n", order_number, books[i].ISBN, "unpaid");
            sendto(serverSocket, &order_number, sizeof(int), 0, (struct sockaddr *)clientAddr, addr_size);
            fclose(order_file);
            fclose(book_file);
            return;
        }
    }
    strcpy(status, "not found");
    sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);

    // Close files
    fclose(order_file);
    fclose(book_file);
}

// Function to pay for a book order
void pay_for_book(int serverSocket, struct sockaddr_in *clientAddr, socklen_t addr_size) {
    FILE* order_file;
    int client_order_number, count = 0;
    char status[50];
    orders orders[100];

    // Open order file
    order_file = fopen("orderfile.txt", "r+");
    if(order_file == NULL) {
        printf("Error opening file\n");
        exit(EXIT_FAILURE);
    }

    // Receive order number from client
    recvfrom(serverSocket, &client_order_number, sizeof(int), 0, (struct sockaddr *)clientAddr, &addr_size);

    // Read orders from file into array
    while (fscanf(order_file, "%d\t%d\t%[^\n]", &orders[count].order_number, &orders[count].ISBN, orders[count].status) != EOF) {
        count++;
    }

    // Find order with matching order number and mark as paid
    for (int i = 0; i < count; i++) {
        if (client_order_number == orders[i].order_number) {
            strcpy(status, "found");
            sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);
            strcpy(orders[i].status, "paid");
            fseek(order_file, 0, SEEK_SET);
            for (int i = 0; i < count; i++) {
                fprintf(order_file, "%d\t%d\t%s\n", orders[i].order_number, orders[i].ISBN, orders[i].status);
            }
            fclose(order_file);
            return;
        }
    }
    strcpy(status, "not found");
    sendto(serverSocket, status, 50, 0, (struct sockaddr *)clientAddr, addr_size);

    // Close file
    fclose(order_file);
}
