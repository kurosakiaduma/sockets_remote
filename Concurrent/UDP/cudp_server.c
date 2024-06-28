#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 4455   // Port number for server communication
#define BUFSIZE 1024 // Buffer size for receiving data

// Structure to represent a book
typedef struct book {
    int Serial_Number;
    char Title[50];
    char Authors[50];
    int ISBN;
    char Publisher[50];
    char Date_of_Publication[50];
} book;

// Structure to represent an order
typedef struct order {
    int order_number;
    int ISBN;
    char status[50];
} order;

// Structure to hold client information for thread handling
typedef struct client_info {
    int serverSocket;
    struct sockaddr_in clientAddr;
    socklen_t addr_size;
    int choice;
} client_info;

// Function declarations
void *handle_client(void *arg);
void display_catalog(client_info *clientData);
void search_book(client_info *clientData);
void order_book(client_info *clientData);
void pay_for_book(client_info *clientData);

int main() {
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size;
    pthread_t tid;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address to bind to any available IP on the specified port
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to server address
    if (bind(sockfd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Allocate memory for client information and receive client's choice
        client_info *clientData = (client_info *)malloc(sizeof(client_info));
        clientData->serverSocket = sockfd;
        addr_size = sizeof(clientAddr);
        clientData->addr_size = addr_size;

        // Receive client's choice
        ssize_t recv_len = recvfrom(sockfd, &clientData->choice, sizeof(int), 0, (struct sockaddr *)&clientAddr, &addr_size);
        if (recv_len < 0) {
            perror("Receive failed");
            free(clientData);
            continue;
        }
        clientData->clientAddr = clientAddr;

        printf("Received choice: %d\n", clientData->choice);

        // Create a new thread to handle the client request
        pthread_create(&tid, NULL, handle_client, clientData);
    }

    close(sockfd);
    return 0;
}

// Thread function to handle client request
void *handle_client(void *arg) {
    client_info *clientData = (client_info *)arg;

    // Determine client's choice and call appropriate function
    switch (clientData->choice) {
        case 1:
            display_catalog(clientData);
            break;
        case 2:
            search_book(clientData);
            break;
        case 3:
            order_book(clientData);
            break;
        case 4:
            pay_for_book(clientData);
            break;
        default:
            printf("Invalid choice received.\n");
            break;
    }

    // Free allocated memory for client data and exit thread
    free(clientData);
    pthread_exit(NULL);
}

// Function to send book catalog to client
void display_catalog(client_info *clientData) {
    book books[100];
    int count = 0;

    // Open file containing book catalog for reading
    FILE *file = fopen("bookfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Read each book record from file and store in books array
    while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }
    fclose(file);

    // Send the number of books in the catalog to the client
    printf("Sending catalog count: %d\n", count);
    sendto(clientData->serverSocket, &count, sizeof(int), 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);

    // Send each book record to the client
    for (int i = 0; i < count; i++) {
        printf("Sending book %d\n", i+1);
        sendto(clientData->serverSocket, &books[i], sizeof(book), 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
    }
}

// Function to search for a book based on client's criteria
void search_book(client_info *clientData) {
    int choice, count;
    char status[50];
    book book[100];

    // Receive search criteria choice from client
    recvfrom(clientData->serverSocket, &choice, sizeof(int), 0, (struct sockaddr *)&clientData->clientAddr, &clientData->addr_size);
    printf("Search choice: %d\n", choice);

    if (choice == 1) {
        char title[50];
        recvfrom(clientData->serverSocket, title, 50, 0, (struct sockaddr *)&clientData->clientAddr, &clientData->addr_size);
        printf("Searching for book by title: %s\n", title);

        // Open book catalog file for reading
        FILE *file = fopen("bookfile.txt", "r");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }

        // Search for book by title and send status and book details to client
        while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &book[count].Serial_Number, book[count].Title, book[count].Authors, &book[count].ISBN, book[count].Publisher, book[count].Date_of_Publication) != EOF) {
            if (strcmp(book[count].Title, title) == 0) {
                strcpy(status, "found");
                sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
                sendto(clientData->serverSocket, &book, sizeof(book), 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
                fclose(file);
                return;
            }
        }

        fclose(file);
        strcpy(status, "not found");
        sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
    } else if (choice == 2) {
        int ISBN;
        recvfrom(clientData->serverSocket, &ISBN, sizeof(int), 0, (struct sockaddr *)&clientData->clientAddr, &clientData->addr_size);
        printf("Searching for book by ISBN: %d\n", ISBN);

        // Open book catalog file for reading
        FILE *file = fopen("bookfile.txt", "r");
        if (file == NULL) {
            perror("Error opening file");
            return;
        }

        // Search for book by ISBN and send status and book details to client
        while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &book[count].Serial_Number, book[count].Title, book[count].Authors, &book[count].ISBN, book[count].Publisher, book[count].Date_of_Publication) != EOF) {
            if (book[count].ISBN == ISBN) {
                strcpy(status, "found");
                sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
                sendto(clientData->serverSocket, &book, sizeof(book), 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
                fclose(file);
                return;
            }
        }

        fclose(file);
        strcpy(status, "not found");
        sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
    }
}

// Function to process book order request from client
void order_book(client_info *clientData) {
    int ISBN;
    char status[50];
    order new_order;
    static int order_number = 1;

    // Receive ISBN of book to be ordered from client
    recvfrom(clientData->serverSocket, &ISBN, sizeof(int), 0, (struct sockaddr *)&clientData->clientAddr, &clientData->addr_size);
    printf("Received ISBN for ordering: %d\n", ISBN);

    // Open book catalog file for reading
    FILE *catalog_file = fopen("bookfile.txt", "r");
    if (catalog_file == NULL) {
        perror("Error opening catalog file");
        return;
    }

    book found_book;
    while (fscanf(catalog_file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &found_book.Serial_Number, found_book.Title, found_book.Authors, &found_book.ISBN, found_book.Publisher, found_book.Date_of_Publication) != EOF) {
        // If book with specified ISBN is found, update status and send confirmation to client
        if (found_book.ISBN == ISBN) {
            strcpy(status, "found");
            sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);

            // Open orders file for appending new order
            FILE *order_file = fopen("orders.txt", "a");
            if (order_file == NULL) {
                perror("Error opening orders file");
                fclose(catalog_file);
                return;
            }

            // Create new order record and write to orders file
            new_order.order_number = order_number++;
            new_order.ISBN = ISBN;
            strcpy(new_order.status, "ordered");
            fprintf(order_file, "%d\t%d\t%s\n", new_order.order_number, new_order.ISBN, new_order.status);
            fclose(order_file);

            // Send order number to client
            sendto(clientData->serverSocket, &new_order.order_number, sizeof(int), 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
            fclose(catalog_file);
            return;
        }
    }

    // If book with specified ISBN is not found, send status to client
    strcpy(status, "not found");
    sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
    fclose(catalog_file);
}

// Function to process payment for an order
void pay_for_book(client_info *clientData) {
    int order_number;
    char status[50];
    order current_order;

    // Receive order number for payment from client
    recvfrom(clientData->serverSocket, &order_number, sizeof(int), 0, (struct sockaddr *)&clientData->clientAddr, &clientData->addr_size);
    printf("Received order number for payment: %d\n", order_number);

    // Open orders file for reading and writing
    FILE *order_file = fopen("orders.txt", "r+");
    if (order_file == NULL) {
        perror("Error opening orders file");
        return;
    }

    // Search for order with specified order number and update status to "paid"
    while (fscanf(order_file, "%d\t%d\t%[^\n]", &current_order.order_number, &current_order.ISBN, current_order.status) != EOF) {
        if (current_order.order_number == order_number) {
            strcpy(status, "found");
            sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);

            // Update status to "paid" in orders file
            fseek(order_file, -strlen(current_order.status), SEEK_CUR);
            strcpy(current_order.status, "paid");
            fprintf(order_file, "%s\n", current_order.status);
            fclose(order_file);
            return;
        }
    }

    // If order with specified order number is not found, send status to client
    strcpy(status, "not found");
    sendto(clientData->serverSocket, status, 50, 0, (struct sockaddr *)&clientData->clientAddr, clientData->addr_size);
    fclose(order_file);
}
