#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
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
void* handle_client(void* arg);

// Structure for representing a book
typedef struct book {
    int Serial_Number;
    char Title[50];
    char Authors[50];
    int ISBN;
    char Publisher[50];
    char Date_of_Publication[50];
} book;

// Structure for representing an order
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

    // Create a socket for communication
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // Initialize server address structure
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    // Listen for incoming connections
    if (listen(sockfd, 5) < 0) {
        perror("listen");
        close(sockfd);
        exit(1);
    }
    
    addr_size = sizeof(newAddr);
    printf("[+]Listening...\n");

    // Accept and handle incoming client connections
    while (1) {
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
        if (newSocket < 0) {
            perror("accept");
            continue;
        }

        // Create a new thread to handle each client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)(long)newSocket) != 0) {
            perror("pthread_create");
            close(newSocket);
            continue;
        }

        // Detach the thread to handle its own cleanup
        pthread_detach(thread_id);
    }
    
    close(sockfd);
    return 0;
}

// Function executed by each thread to handle client requests
void* handle_client(void* arg) {
    int newSocket = (int)(long)arg;
    printf("[+]Client connected\n");

    int choice;
    // Read the choice of action from the client
    if (read(newSocket, &choice, sizeof(int)) <= 0) {
        perror("read");
        close(newSocket);
        pthread_exit(NULL);
    }
    printf("Data Received: %d\n", choice);

    // Handle client request based on the choice
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
            // Close the connection for choice 5 (Exit)
            close(newSocket);
            pthread_exit(NULL);
            break;
        default:
            printf("Invalid choice\n");
            break;
    }

    // Close the socket and exit the thread
    close(newSocket);
    pthread_exit(NULL);
}

// Function to display the catalog of books to the client
void display_catalog(int clientSocket) {
    FILE* file;
    book books[100];
    int count = 0;

    // Open the file containing book information
    file = fopen("bookfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Read book details from file into memory
    while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }
    fclose(file);

    // Send the number of books and book details to the client
    write(clientSocket, &count, sizeof(int));
    write(clientSocket, books, sizeof(book) * count);
}

// Function to search for a book by title or ISBN
void search_book(int newSocket) {
    FILE* file;
    book books[100];
    int count = 0, choice;
    char status[50];

    // Open the file containing book information
    file = fopen("bookfile.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Read book details from file into memory
    while (fscanf(file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }
    fclose(file);

    // Read the client's search choice
    if (read(newSocket, &choice, sizeof(int)) <= 0) {
        perror("read");
        return;
    }

    switch (choice) {
        case 1: {
            char title[50];
            // Read the book title to search for from the client
            if (read(newSocket, title, sizeof(title)) <= 0) {
                perror("read");
                return;
            }
            // Search for the book by title in memory
            for (int i = 0; i < count; i++) {
                if (strcmp(title, books[i].Title) == 0) {
                    strcpy(status, "found");
                    write(newSocket, status, sizeof(status));
                    write(newSocket, &books[i], sizeof(book));
                    return;
                }
            }
            strcpy(status, "not found");
            write(newSocket, status, sizeof(status));
            break;
        }
        case 2: {
            int ISBN;
            // Read the book ISBN to search for from the client
            if (read(newSocket, &ISBN, sizeof(ISBN)) <= 0) {
                perror("read");
                return;
            }
            // Search for the book by ISBN in memory
            for (int i = 0; i < count; i++) {
                if (ISBN == books[i].ISBN) {
                    strcpy(status, "found");
                    write(newSocket, status, sizeof(status));
                    write(newSocket, &books[i], sizeof(book));
                    return;
                }
            }
            strcpy(status, "not found");
            write(newSocket, status, sizeof(status));
            break;
        }
    }
}

// Function to process a book order from the client
void order_book(int newSocket) {
    srand(time(NULL));
    FILE* book_file;
    FILE* order_file;
    book books[100];
    int count = 0, order_number, ISBN;
    char status[50];

    // Open the file containing book information
    book_file = fopen("bookfile.txt", "r");
    if (book_file == NULL) {
        perror("Error opening book file");
        return;
    }

    // Open the file to store order information
    order_file = fopen("orderfile.txt", "a");
    if (order_file == NULL) {
        perror("Error opening order file");
        fclose(book_file);
        return;
    }

    // Read book details from file into memory
    if (read(newSocket, &ISBN, sizeof(ISBN)) <= 0) {
        perror("read");
        fclose(book_file);
        fclose(order_file);
        return;
    }
    while (fscanf(book_file, "%d\t%[^\t]\t%[^\t]\t%d\t%[^\t]\t%[^\n]", &books[count].Serial_Number, books[count].Title, books[count].Authors, &books[count].ISBN, books[count].Publisher, books[count].Date_of_Publication) != EOF) {
        count++;
    }
    fclose(book_file);

    // Search for the book by ISBN and process the order
    for (int i = 0; i < count; i++) {
        if (ISBN == books[i].ISBN) {
            strcpy(status, "found");
            write(newSocket, status, sizeof(status));
            order_number = rand() % 1000;
            fprintf(order_file, "%d\t%d\t%s\n", order_number, books[i].ISBN, "unpaid");
            write(newSocket, &order_number, sizeof(order_number));
            fclose(order_file);
            return;
        }
    }
    strcpy(status, "not found");
    write(newSocket, status, sizeof(status));
    fclose(order_file);
}

// Function to process payment for a book order from the client
void pay_for_book(int newSocket) {
    FILE* order_file;
    int client_order_number, count = 0;
    char status[50];
    orders order_list[100];

    // Open the file containing order information
    order_file = fopen("orderfile.txt", "r+");
    if (order_file == NULL) {
        perror("Error opening file");
        return;
    }

    // Read the order number from the client
    if (read(newSocket, &client_order_number, sizeof(client_order_number)) <= 0) {
        perror("read");
        fclose(order_file);
        return;
    }

    // Read order details from file into memory
    while (fscanf(order_file, "%d\t%d\t%[^\n]", &order_list[count].order_number, &order_list[count].ISBN, order_list[count].status) != EOF) {
        count++;
    }

    // Process payment for the specified order number
    for (int i = 0; i < count; i++) {
        if (client_order_number == order_list[i].order_number) {
            strcpy(status, "found");
            write(newSocket, status, sizeof(status));
            strcpy(order_list[i].status, "paid");
            fclose(order_file);

            // Rewrite updated order information to file
            order_file = fopen("orderfile.txt", "w");
            for (int j = 0; j < count; j++) {
                fprintf(order_file, "%d\t%d\t%s\n", order_list[j].order_number, order_list[j].ISBN, order_list[j].status);
            }
            fclose(order_file);
            return;
        }
    }
    strcpy(status, "not found");
    write(newSocket, status, sizeof(status));
    fclose(order_file);
}
