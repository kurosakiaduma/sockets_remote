#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 4455
#define BUFSIZE 1024

// Function prototypes
void display_catalog(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);
void search_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);
void order_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);
void pay_for_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);

// Structure for book details
typedef struct book {
    int Serial_Number;
    char Title[50];
    char Authors[50];
    int ISBN;
    char Publisher[50];
    char Date_of_Publication[50];
} book;

// Main function for the client application
int main() {
    int sockfd;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    int choice;

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
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Localhost IP address

    addr_size = sizeof(serverAddr);

    // Main menu loop
    while (1) {
        printf("1. Display book catalog\n");
        printf("2. Search for a book\n");
        printf("3. Order a book\n");
        printf("4. Pay for a book\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Send user's choice to the server
        sendto(sockfd, &choice, sizeof(int), 0, (struct sockaddr *)&serverAddr, addr_size);

        // Switch case to handle user choices
        switch (choice) {
            case 1:
                display_catalog(sockfd, &serverAddr, addr_size);
                break;
            case 2:
                search_book(sockfd, &serverAddr, addr_size);
                break;
            case 3:
                order_book(sockfd, &serverAddr, addr_size);
                break;
            case 4:
                pay_for_book(sockfd, &serverAddr, addr_size);
                break;
            case 5:
                close(sockfd);
                exit(EXIT_SUCCESS);  // Exit the program
                break;
            default:
                printf("Invalid choice\n");
                break;
        }
    }

    return 0;
}

// Function to display catalog of books
void display_catalog(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size) {
    int count;
    book books[100];

    // Receive number of books from server
    recvfrom(clientSocket, &count, sizeof(int), 0, (struct sockaddr *)serverAddr, &addr_size);
    sendto(clientSocket, &count, sizeof(int), 0, (struct sockaddr *)serverAddr, addr_size);

    // Receive and display each book from server
    for (int i = 0; i < count; i++) {
        recvfrom(clientSocket, &books[i], sizeof(book), 0, (struct sockaddr *)serverAddr, &addr_size);
        printf("Serial Number: %d\nTitle: %s\nAuthors: %s\nISBN: %d\nPublisher: %s\nDate of Publication: %s\n\n",
               books[i].Serial_Number, books[i].Title, books[i].Authors, books[i].ISBN, books[i].Publisher, books[i].Date_of_Publication);
    }
}

// Function to search for a book by title or ISBN
void search_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size) {
    int choice;
    char status[50];
    book found_book;

    // User chooses search type (by title or ISBN)
    printf("Search book by: \n1. Title \n2. ISBN\n");
    scanf("%d", &choice);
    sendto(clientSocket, &choice, sizeof(int), 0, (struct sockaddr *)serverAddr, addr_size);

    switch (choice) {
        case 1:
            {
                char title[50];
                printf("Enter title: ");
                scanf("%s", title);
                sendto(clientSocket, title, 50, 0, (struct sockaddr *)serverAddr, addr_size);
                recvfrom(clientSocket, status, 50, 0, (struct sockaddr *)serverAddr, &addr_size);

                if (strcmp(status, "found") == 0) {
                    recvfrom(clientSocket, &found_book, sizeof(book), 0, (struct sockaddr *)serverAddr, &addr_size);
                    printf("Serial Number: %d\nTitle: %s\nAuthors: %s\nISBN: %d\nPublisher: %s\nDate of Publication: %s\n\n",
                           found_book.Serial_Number, found_book.Title, found_book.Authors, found_book.ISBN, found_book.Publisher, found_book.Date_of_Publication);
                } else {
                    printf("Book not found\n");
                }
            }
            break;
        case 2:
            {
                int ISBN;
                printf("Enter ISBN: ");
                scanf("%d", &ISBN);
                sendto(clientSocket, &ISBN, sizeof(int), 0, (struct sockaddr *)serverAddr, addr_size);
                recvfrom(clientSocket, status, 50, 0, (struct sockaddr *)serverAddr, &addr_size);

                if (strcmp(status, "found") == 0) {
                    recvfrom(clientSocket, &found_book, sizeof(book), 0, (struct sockaddr *)serverAddr, &addr_size);
                    printf("Serial Number: %d\nTitle: %s\nAuthors: %s\nISBN: %d\nPublisher: %s\nDate of Publication: %s\n\n",
                           found_book.Serial_Number, found_book.Title, found_book.Authors, found_book.ISBN, found_book.Publisher, found_book.Date_of_Publication);
                } else {
                    printf("Book not found\n");
                }
            }
            break;
    }
}

// Function to order a book
void order_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size) {
    int ISBN, order_number;
    char status[50];

    // User enters ISBN of the book to order
    printf("Enter ISBN of the book you want to order: ");
    scanf("%d", &ISBN);
    sendto(clientSocket, &ISBN, sizeof(int), 0, (struct sockaddr *)serverAddr, addr_size);
    recvfrom(clientSocket, status, 50, 0, (struct sockaddr *)serverAddr, &addr_size);

    // If book is found, receive order number from server
    if (strcmp(status, "found") == 0) {
        recvfrom(clientSocket, &order_number, sizeof(int), 0, (struct sockaddr *)serverAddr, &addr_size);
        printf("Order successful. Your order number is: %d\n", order_number);
    } else {
        printf("Book not found\n");
    }
}

// Function to pay for a book order
void pay_for_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size) {
    int order_number;
    char status[50];

    // User enters order number to pay for
    printf("Enter your order number: ");
    scanf("%d", &order_number);
    sendto(clientSocket, &order_number, sizeof(int), 0, (struct sockaddr *)serverAddr, addr_size);
    recvfrom(clientSocket, status, 50, 0, (struct sockaddr *)serverAddr, &addr_size);

    // If order is found, confirm payment
    if (strcmp(status, "found") == 0) {
        printf("Payment successful. Your order is now paid.\n");
    } else {
        printf("Order not found\n");
    }
}
