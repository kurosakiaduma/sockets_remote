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

// Function prototypes
void display_catalog(int clientSocket);
void search_book(int newSocket);
void order_book(int newSocket);
void pay_for_book(int newSocket);
void handle_client(int newSocket);

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

int main() {
    int sockfd;
    struct sockaddr_in serverAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;
    
    // Buffer for communication
    char buffer[1024];
    
    // Create socket
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");  // Listen on all network interfaces

    // Bind socket to address
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(sockfd, 5);
    addr_size = sizeof(newAddr);
    printf("[+]Listening...\n");

    // Accept incoming connections and handle clients
    while (1) {
        if ((newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        handle_client(newSocket);
        close(newSocket);
    }

    return 0;
}

// Function to handle client requests
void handle_client(int newSocket) {
    printf("[+]Client connected\n");
    int choice;
    read(newSocket, &choice, sizeof(int));  // Read client's choice
    printf("Data Received: %d\n", choice);

    // Process client's choice
    switch (choice) {
        case 1:
            display_catalog(newSocket);
            break;
        case 2:
            search_book(newSocket);
            break;
        case 3:
            order_book(newSocket);
            break;
        case 4:
            pay_for_book(newSocket);
            break;
        case 5:
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("Invalid choice\n");
            break;
    }
}

// Function to display catalog of books
void display_catalog(int clientSocket) {
    FILE* file;
    book books[100];  // Array to store books
    int book_count = 0, count = 0;
    
    // Open book file for reading
    file = fopen("bookfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Read books from file
    while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }

    // Send number of books to client
    book_count = count;
    write(clientSocket, &book_count, sizeof(int));

    // Receive number of books client wants to see
    read(clientSocket, &book_count, sizeof(int));

    // Send requested books to client
    for (int i = 0; i < book_count; i++) {
        write(clientSocket, &books[i], sizeof(book));
    }

    fclose(file);
}

// Function to search for a book by title or ISBN
void search_book(int newSocket) {
    FILE* file;
    book books[100];  // Array to store books
    int count = 0, choice;
    char status[50];

    // Open book file for reading
    file = fopen("bookfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Read books from file
    while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }

    // Receive client's search choice
    read(newSocket, &choice, sizeof(int));

    // Process based on client's choice
    switch (choice) {
        case 1:
            char title[50];
            read(newSocket, title, 50);  // Receive title from client
            for (int i = 0; i < count; i++) {
                if (strcmp(title, books[i].Title) == 0) {
                    strcpy(status, "found");
                    write(newSocket, status, 50);  // Send status to client
                    write(newSocket, &books[i], sizeof(book));  // Send book details to client
                    break;
                }
            }
            strcpy(status, "not found");
            write(newSocket, status, 50);  // Send status to client
            break;
        case 2:
            int ISBN;
            read(newSocket, &ISBN, sizeof(int));  // Receive ISBN from client
            for (int i = 0; i < count; i++) {
                if (ISBN == books[i].ISBN) {
                    strcpy(status, "found");
                    write(newSocket, status, 50);  // Send status to client
                    write(newSocket, &books[i], sizeof(book));  // Send book details to client
                    break;
                }
            }
            strcpy(status, "not found");
            write(newSocket, status, 50);  // Send status to client
            break;
    }

    fclose(file);
}

// Function to process book ordering
void order_book(int newSocket) {
    srand(time(NULL));  // Seed random number generator
    FILE* book_file;
    FILE* order_file;
    book books[100];  // Array to store books
    int count = 0, order_number, ISBN;
    char status[50];

    // Open book file for reading
    book_file = fopen("bookfile.txt", "r");
    order_file = fopen("orderfile.txt", "a");
    if (book_file == NULL || order_file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Receive ISBN from client
    read(newSocket, &ISBN, sizeof(int));

    // Read books from file
    while (fscanf(book_file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }

    fclose(book_file);

    // Find book with given ISBN and process order
    for (int i = 0; i < count; i++) {
        if (ISBN == books[i].ISBN) {
            strcpy(status, "found");
            write(newSocket, status, 50);  // Send status to client
            order_number = rand() % 1000;  // Generate random order number
            fprintf(order_file, "%d\t%d\t%s\n", order_number, books[i].ISBN, "unpaid");  // Write order details to order file
            write(newSocket, &order_number, sizeof(int));  // Send order number to client
            fclose(order_file);
            break;
        }
    }

    if (strcmp(status, "not found") != 0) {
        strcpy(status, "not found");
        write(newSocket, status, 50);  // Send status to client
    }
}

// Function to process payment for a book order
void pay_for_book(int newSocket) {
    FILE* order_file;
    int client_order_number, count = 0;
    char status[50];
    orders orders[100];  // Array to store orders

    // Open order file for reading and writing
    order_file = fopen("orderfile.txt", "r+");
    if (order_file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Receive order number from client
    read(newSocket, &client_order_number, sizeof(int));

    // Read orders from file
    while (fscanf(order_file, "%d\t%d\t%[^\n]", &orders[count].order_number, &orders[count].ISBN, orders[count].status) != EOF) {
        count++;
    }

    // Process payment for the given order number
    for (int i = 0; i < count; i++) {
        if (client_order_number == orders[i].order_number) {
            strcpy(status, "found");
            write(newSocket, status, 50);  // Send status to client
            strcpy(orders[i].status, "paid");  // Update order status to paid
            fseek(order_file, 0, SEEK_SET);  // Move file pointer to beginning
            for (int j = 0; j < count; j++) {
                fprintf(order_file, "%d\t%d\t%s\n", orders[j].order_number, orders[j].ISBN, orders[j].status);  // Write updated orders to file
            }
            fclose(order_file);
            break;
        }
    }

    if (strcmp(status, "not found") != 0) {
        strcpy(status, "not found");
        write(newSocket, status, 50);  // Send status to client
    }
}
