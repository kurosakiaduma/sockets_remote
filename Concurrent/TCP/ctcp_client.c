#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 4455

// Define a structure for representing a book
typedef struct book {
    int Serial_Number;
    char Title[50];
    char Authors[50];
    int ISBN;
    char Publisher[50];
    char Date_of_Publication[50];
} book;

// Function prototypes
void display_catalog(int clientSocket);
void search_book(int clientSocket);
void order_book(int clientSocket);
void pay_for_book(int clientSocket);

int main() {
    int clientSocket, choice;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    // Create a socket for communication
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);

    // Initialize server address structure
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        printf("Connection failed\n");
        exit(1);
    }
    printf("[+]Connected to Server\n");

    // Displaying the menu to the user
    printf("Welcome to The Online Bookshop\n");
    printf("================================\n");

    while(1) {
        printf("Please select an option:\n");
        printf("1. Display Catalog\n");
        printf("2. Search for a book\n");
        printf("3. Order book\n");
        printf("4. Pay for book\n");
        printf("5. Exit\n");
        scanf("%d", &choice);

        // Send user choice to server
        write(clientSocket, &choice, sizeof(choice));

        // Handle user choice based on the selected menu option
        switch(choice) {
            case 1:
                display_catalog(clientSocket);
                break;
            case 2:
                search_book(clientSocket);
                break;
            case 3:
                order_book(clientSocket);
                break;
            case 4:
                pay_for_book(clientSocket);
                break;
            case 5:
                // Close the connection and exit
                close(clientSocket);
                printf("Thanks for visiting The Online Bookshop\n");
                exit(1);
            default:
                printf("Invalid choice\n");
                continue;
        }
    }
}

// Function to display the catalog of books
void display_catalog(int clientSocket) {
    int book_count;
    char temp;

    // Receive the number of books in the catalog from the server
    read(clientSocket, &book_count, sizeof(book_count));
    printf("We've got %d books in our catalog\n", book_count);

    // Prompt user to specify how many books they want to see
    printf("How many books would you like to see: ");
    scanf("%d", &book_count);
    scanf("%c", &temp);

    // Send the requested number of books to the client
    write(clientSocket, &book_count, sizeof(book_count));

    // Receive and display each book's details
    for(int i = 0; i < book_count; i++) {
        book book;
        read(clientSocket, &book, sizeof(book));
        printf("S/No.\tTitle\t\tAuthor\t\tISBN\tPublisher\t\tDate of Publication\n");
        printf("%d\t%s\t%s\t%d\t%s\t%s\n", book.Serial_Number, book.Title, book.Authors, book.ISBN, book.Publisher, book.Date_of_Publication);
        printf("================================\n");
    }
}

// Function to search for a book by title or ISBN
void search_book(int clientSocket) {
    int choice, ISBN;
    char Title[50], status[50], temp;
    book book;

    // Display search options
    printf("Please select an option:\n");
    printf("1. Search by Title\n");
    printf("2. Search by ISBN\n");
    scanf("%d", &choice);
    scanf("%c", &temp);

    switch (choice) {
        case 1:
            printf("Enter the book's Title: ");
            scanf("%[^\n]", Title);
            scanf("%c", &temp);

            // Send search choice and title to server
            write(clientSocket, &choice, sizeof(choice));
            write(clientSocket, &Title, sizeof(Title));

            // Receive search status from server
            read(clientSocket, &status, sizeof(status));

            // Handle book found or not found
            if (strcmp(status, "not found") == 0) {
                printf("Book not found\n");
                break;
            }
            // Receive and display book details
            read(clientSocket, &book, sizeof(book));
            printf("S/No.\tTitle\t\tAuthor\t\tISBN\tPublisher\t\tDate of Publication\n");
            printf("%d\t%s\t%s\t%d\t%s\t%s\n", book.Serial_Number, book.Title, book.Authors, book.ISBN, book.Publisher, book.Date_of_Publication);
            break;

        case 2:
            printf("Enter the book's ISBN: ");
            scanf("%d", &ISBN);
            scanf("%c", &temp);

            // Send search choice and ISBN to server
            write(clientSocket, &choice, sizeof(choice));
            write(clientSocket, &ISBN, sizeof(ISBN));

            // Receive search status from server
            read(clientSocket, &status, sizeof(status));

            // Handle book found or not found
            if (strcmp(status, "not found") == 0) {
                printf("Book not found\n");
                break;
            }
            // Receive and display book details
            read(clientSocket, &book, sizeof(book));
            printf("S/No.\tTitle\t\tAuthor\t\tISBN\tPublisher\t\tDate of Publication\n");
            printf("%d\t%s\t%s\t%d\t%s\t%s\n", book.Serial_Number, book.Title, book.Authors, book.ISBN, book.Publisher, book.Date_of_Publication);
            break;

        default:
            printf("Invalid choice\n");
            break;
    }
}

// Function to order a book by ISBN
void order_book(int clientSocket) {
    int ISBN, order_number;
    char temp, status[50];

    // Prompt user to enter ISBN of the book to order
    printf("Enter the ISBN of the book to order: ");
    scanf("%d", &ISBN);
    scanf("%c", &temp);

    // Send ISBN to server for ordering
    write(clientSocket, &ISBN, sizeof(ISBN));

    // Receive order status from server
    read(clientSocket, &status, sizeof(status));

    // Handle book not found scenario
    if (strcmp(status, "not found") == 0) {
        printf("Book not found\n");
        return;
    }

    // Receive order number from server and display
    read(clientSocket, &order_number, sizeof(order_number));
    printf("Book ordered successfully\n");
    printf("Your order number is %d\n", order_number);
    printf("Please keep this number for reference\n");
}

// Function to pay for a book using order number
void pay_for_book(int clientSocket) {
    int order_number;
    char temp, status[50];

    // Prompt user to enter order number for payment
    printf("Enter the order number: ");
    scanf("%d", &order_number);
    scanf("%c", &temp);

    // Send order number to server for payment
    write(clientSocket, &order_number, sizeof(order_number));

    // Receive payment status from server
    read(clientSocket, &status, sizeof(status));

    // Handle order not found scenario
    if (strcmp(status, "not found") == 0) {
        printf("Order not found\n");
        return;
    }

    // Display payment successful message
    printf("Payment successful\n");
    printf("Thank you for shopping with us\n");
}
