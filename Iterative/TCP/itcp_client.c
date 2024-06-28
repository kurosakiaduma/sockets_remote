#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 4455

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

    // Create a socket for communication with the server
    clientSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address parameters
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.88.148");  // Replace with your server's IP address

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        printf("Connection failed\n");
        exit(EXIT_FAILURE);
    }
    printf("[+]Connected to Server\n");

    // Display menu and handle user input
    printf("Welcome to The Online Bookshop\n");
    printf("================================\n");
    while (1) {
        printf("Please select an option:\n");
        printf("1. Display Catalog\n");
        printf("2. Search for a book\n");
        printf("3. Order book\n");
        printf("4. Pay for book\n");
        printf("5. Exit\n");
        scanf("%d", &choice);

        // Send user's choice to the server
        write(clientSocket, &choice, sizeof(choice));

        // Perform corresponding action based on user's choice
        switch (choice) {
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
                close(clientSocket);
                printf("Thanks for visiting The Online Bookshop\n");
                exit(EXIT_SUCCESS);
            default:
                printf("Invalid choice\n");
                break;
        }
    }
}

// Function to display catalog of books
void display_catalog(int clientSocket) {
    int book_count;
    char temp;

    // Receive the count of books from server
    read(clientSocket, &book_count, sizeof(book_count));
    printf("We've got %d books in our catalog\n", book_count);

    // Prompt user for number of books to display
    printf("How many books would you like to see: ");
    scanf("%d", &book_count);
    scanf("%c", &temp);  // Consume newline character left in input buffer

    // Send number of books user wants to see to server
    write(clientSocket, &book_count, sizeof(book_count));

    // Receive and display each book's details
    for (int i = 0; i < book_count; i++) {
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
    scanf("%c", &temp);  // Consume newline character left in input buffer

    switch (choice) {
        case 1:
            // Search by title
            printf("Enter the book's Title: ");
            scanf("%[^\n]", Title);
            scanf("%c", &temp);  // Consume newline character left in input buffer

            // Send choice and title to server
            write(clientSocket, &choice, sizeof(choice));
            write(clientSocket, &Title, sizeof(Title));

            // Receive search status and book details from server
            read(clientSocket, &status, sizeof(status));
            if (strcmp(status, "not found") == 0) {
                printf("Book not found\n");
                break;
            }
            read(clientSocket, &book, sizeof(book));
            printf("S/No.\tTitle\t\tAuthor\t\tISBN\tPublisher\t\tDate of Publication\n");
            printf("%d\t%s\t%s\t%d\t%s\t%s\n", book.Serial_Number, book.Title, book.Authors, book.ISBN, book.Publisher, book.Date_of_Publication);
            break;
        case 2:
            // Search by ISBN
            printf("Enter the book's ISBN: ");
            scanf("%d", &ISBN);
            scanf("%c", &temp);  // Consume newline character left in input buffer

            // Send choice and ISBN to server
            write(clientSocket, &choice, sizeof(choice));
            write(clientSocket, &ISBN, sizeof(ISBN));

            // Receive search status and book details from server
            read(clientSocket, &status, sizeof(status));
            if (strcmp(status, "not found") == 0) {
                printf("Book not found\n");
                break;
            }
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

    // Prompt user for ISBN of the book to order
    printf("Enter the ISBN of the book to order: ");
    scanf("%d", &ISBN);
    scanf("%c", &temp);  // Consume newline character left in input buffer

    // Send ISBN to server
    write(clientSocket, &ISBN, sizeof(ISBN));

    // Receive order status and order number from server
    read(clientSocket, &status, sizeof(status));
    if (strcmp(status, "not found") == 0) {
        printf("Book not found\n");
        return;
    }
    read(clientSocket, &order_number, sizeof(order_number));
    printf("Book ordered successfully\n");
    printf("Your order number is %d\n", order_number);
    printf("Please keep this number for reference\n");
}

// Function to pay for a book by order number
void pay_for_book(int clientSocket) {
    int order_number;
    char temp, status[50];

    // Prompt user for order number to pay
    printf("Enter the order number: ");
    scanf("%d", &order_number);
    scanf("%c", &temp);  // Consume newline character left in input buffer

    // Send order number to server
    write(clientSocket, &order_number, sizeof(order_number));

    // Receive payment status from server
    read(clientSocket, &status, sizeof(status));
    if (strcmp(status, "not found") == 0) {
        printf("Order not found\n");
        return;
    }
    printf("Payment successful\n");
    printf("Thank you for shopping with us\n");
}
