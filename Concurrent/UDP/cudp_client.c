#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 4455  // Define the port number to be used
#define BUFSIZE 1024  // Define buffer size for receiving data

// Function declarations
void display_catalog(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);
void search_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);
void order_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);
void pay_for_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size);

// Structure to represent a book
typedef struct book {
    int Serial_Number;
    char Title[50];
    char Authors[50];
    int ISBN;
    char Publisher[50];
    char Date_of_Publication[50];
} book;

int main() {
    int sockfd;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    int choice;

    // Create a socket for UDP communication
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Initialize server address structure
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    addr_size = sizeof(serverAddr);
    
    while(1) {
        // Display menu options to the user
        printf("1. Display book catalog\n");
        printf("2. Search for a book\n");
        printf("3. Order a book\n");
        printf("4. Pay for a book\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        // Send the user's choice to the server
        printf("Sending choice %d to server...\n", choice);
        sendto(sockfd, &choice, sizeof(int), 0, (struct sockaddr *)&serverAddr, addr_size);
        printf("Choice sent.\n");
        
        // Handle the user's choice
        switch(choice) {
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
                close(sockfd);  // Close the socket and exit
                exit(1);
                break;
        }
    }
    return 0;
}

// Function to display the book catalog
void display_catalog(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size) {
    int count;
    book books[100];
    
    // Receive the number of books from the server
    printf("Waiting for catalog count...\n");
    recvfrom(clientSocket, &count, sizeof(int), 0, (struct sockaddr *)serverAddr, &addr_size);
    printf("Received count: %d\n", count);
    
    // Receive each book's details from the server
    for (int i = 0; i < count; i++) {
        printf("Waiting for book %d...\n", i+1);
        recvfrom(clientSocket, &books[i], sizeof(book), 0, (struct sockaddr *)serverAddr, &addr_size);
        printf("Received book %d: Serial Number: %d\nTitle: %s\nAuthors: %s\nISBN: %d\nPublisher: %s\nDate of Publication: %s\n\n", i+1, books[i].Serial_Number, books[i].Title, books[i].Authors, books[i].ISBN, books[i].Publisher, books[i].Date_of_Publication);
    }
}

// Function to search for a book
void search_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size) {
    int choice;
    char status[50];
    book found_book;
    
    // Prompt user to choose search method
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
                    printf("Serial Number: %d\nTitle: %s\nAuthors: %s\nISBN: %d\nPublisher: %s\nDate of Publication: %s\n\n", found_book.Serial_Number, found_book.Title, found_book.Authors, found_book.ISBN, found_book.Publisher, found_book.Date_of_Publication);
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
                    printf("Serial Number: %d\nTitle: %s\nAuthors: %s\nISBN: %d\nPublisher: %s\nDate of Publication: %s\n\n", found_book.Serial_Number, found_book.Title, found_book.Authors, found_book.ISBN, found_book.Publisher, found_book.Date_of_Publication);
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
    
    // Prompt user for ISBN of the book to order
    printf("Enter ISBN of the book you want to order: ");
    scanf("%d", &ISBN);
    sendto(clientSocket, &ISBN, sizeof(int), 0, (struct sockaddr *)serverAddr, addr_size);
    recvfrom(clientSocket, status, 50, 0, (struct sockaddr *)serverAddr, &addr_size);
    
    if (strcmp(status, "found") == 0) {
        recvfrom(clientSocket, &order_number, sizeof(int), 0, (struct sockaddr *)serverAddr, &addr_size);
        printf("Order successful. Your order number is: %d\n", order_number);
    } else {
        printf("Book not found\n");
    }
}

// Function to pay for a book
void pay_for_book(int clientSocket, struct sockaddr_in *serverAddr, socklen_t addr_size) {
    int order_number;
    char status[50];
    
    // Prompt user for order number to process payment
    printf("Enter your order number: ");
    scanf("%d", &order_number);
    sendto(clientSocket, &order_number, sizeof(int), 0, (struct sockaddr *)serverAddr, addr_size);
    recvfrom(clientSocket, status, 50, 0, (struct sockaddr *)serverAddr, &addr_size);
    
    if (strcmp(status, "found") == 0) {
        printf("Payment successful. Your order is now paid.\n");
    } else {
        printf("Order not found\n");
    }
}
