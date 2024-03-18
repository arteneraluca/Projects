#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
    int idThread; // id-ul thread-ului tinut in evidenta de acest program
    int cl;       // descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

void categories(char *raspuns);
void products(char *category, char *raspuns);
void add_to_cart(char *product, char *raspuns);
void show_cart(int socket, char *raspuns);
int exit_function();
int buy_cart();
char usernameCopy[150] = " ";

int check_username(char *username)
{
    // Deschidem fisierul cu credentiale
    FILE *credentials_fd = fopen("credentials.txt", "r");
    if (!credentials_fd)
    {
        printf("[server] Eroare la deschiderea fisierului cu credentiale\n");
        return 0; // Autentificare esuata
    }

    // Cautam user-ul in fisier
    char line[100];
    while (fgets(line, sizeof(line), credentials_fd))
    {
        char *saved_username = strtok(line, " \n");

        // Verificam daca user-ul si parola sunt valide
        if (strcmp(username, saved_username) == 0)
        {
            printf("[server] %s - verificare\n", username);
            fclose(credentials_fd);
            return 1; // Autentificare reusita
        }
    }

    fclose(credentials_fd);
    return 0; // Autentificare esuata
}

int check_password(char *username, char *password)
{
    // Deschidem fisierul cu credentiale
    FILE *credentials_fd = fopen("credentials.txt", "r");
    if (!credentials_fd)
    {
        printf("[server] Eroare la deschiderea fisierului cu credentiale\n");
        return 0; // Autentificare esuata
    }

    // Cautam parola in fisier
    char line[100];
    while (fgets(line, sizeof(line), credentials_fd))
    {
        char *saved_username = strtok(line, " \n");
        char *saved_password = strtok(NULL, " \n");

        // Verificam daca user-ul si parola sunt valide
        if (strcmp(username, saved_username) == 0 &&
            strcmp(password, saved_password) == 0)
        {
            printf("[server] %s - verificare\n", password);
            fclose(credentials_fd);
            return 1; // Autentificare reusita
        }
    }

    fclose(credentials_fd);
    return 0; // Autentificare esuata
}

void login_user(int cl)
{
    int ok = 0;
    int logged_in = 0;
    char mesaj[150], raspuns[150];
    char username[150];

    bzero(&mesaj, sizeof(mesaj));
    bzero(&raspuns, sizeof(raspuns));

    while (logged_in == 0)
    {
        if (ok == 0)
            strcpy(mesaj, "\n>>Username: ");
        else
            strcpy(mesaj, "\n>>Parola: ");

        if (write(cl, mesaj, 150) < 0)
            printf("[server] Eroare la trimiterea mesajului catre [client]\n");

        fflush(stdout);
        bzero(raspuns, sizeof(raspuns));

        if (read(cl, raspuns, 150) < 0)
            printf("[server] eroare la citirea mesajului de la [client]\n");

        raspuns[strlen(raspuns) - 1] = '\0';
        printf("[client] Raspuns: %s\n", raspuns);

        if (ok == 0)
        {
            if (check_username(raspuns) == 1)
            {
                ok = 1; // user corect, trecem la verificarea parolei
                strcpy(username, raspuns);
                strcpy(usernameCopy, username);
                printf("[server] Username corect\n");
            }
        }
        else
        {
            if (check_password(username, raspuns) == 1)
            {
                logged_in = 1;
                printf("[server] Client %d logat cu succes\n", cl);

                bzero(&mesaj, sizeof(mesaj));

                strcpy(mesaj, "\n--Autentificat cu succes--\n");
                if (write(cl, mesaj, 150) < 0)
                {
                    printf("[server] Eroare la trimiterea mesajului catre [client]\n");
                }
            }
        }

        bzero(&mesaj, sizeof(mesaj));
        bzero(&raspuns, sizeof(raspuns));
    }
}

void register_user(int cl)
{
    int ok = 0;
    char mesaj[150], raspuns[150];
    char username[150] = "";

    while (ok != -1)
    {
        bzero(&mesaj, sizeof(mesaj));
        bzero(&raspuns, sizeof(raspuns));

        if (ok == 0)
            strcpy(mesaj, "\n>>Username:");
        else
            strcpy(mesaj, "\n>>Password:");

        if (write(cl, mesaj, 150) < 0)
        {
            printf("[server] Eroare la trimiterea mesajului catre [client]\n");
        }

        fflush(stdout);

        if (read(cl, raspuns, 150) < 0)
        {
            printf("[server] Eroare la citirea mesajului de la [client]\n");
        }

        raspuns[strlen(raspuns) - 1] = '\0';
        printf("[client] Raspuns: %s\n", raspuns);

        if (ok == 0)
        {
            if (check_username(raspuns) == 0) // username nou
            {
                ok = 1;
                strcpy(username, raspuns);
                strcpy(usernameCopy, username);
                printf("[server] Username nou\n");
            }
            else
            {
                printf("[server] Username-ul este deja existent\n");
            }
        }
        else
        {
            ok = -1;
            printf("[server] Client %d autentificat cu succes\n", cl);
            bzero(&mesaj, sizeof(mesaj));

            strcpy(mesaj, "\n--Autentificat cu succes--\n");

            if (write(cl, mesaj, 150) < 0)
            {
                printf("[server] Eroare la trimiterea mesajului catre [client]\n");
            }
        }
    }

    // salvam in fisier noul user
    FILE *credentials_fd = fopen("credentials.txt", "a");
    if (!credentials_fd)
    {
        printf("[server] Eroare la deschiderea fisierului\n");
        exit(0);
    }
    else
    {
        fputs("\n", credentials_fd);
        fputs(username, credentials_fd);
        fputs(" ", credentials_fd);
        fputs(raspuns, credentials_fd);
        printf("[server] Credentiale inregistrate\n");
        fclose(credentials_fd);
    }
}

int main()
{
    struct sockaddr_in server; // structura folosita de server
    struct sockaddr_in from;
    int sd;            // descriptorul de socket
    pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
    int i = 0;

    /* crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    printf("[server]Asteptam la portul %d...\n", PORT);

    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData *td; // parametru functia executata de thread
        int length = sizeof(from);

        fflush(stdout);

        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);
    }
};

static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    fflush(stdout);
    pthread_detach(pthread_self());

    char mesaj[150];
    bzero(&mesaj, sizeof(mesaj));

    if (read(tdL.cl, &mesaj, sizeof(mesaj)) <= 0)
    {
        printf("[client %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }

    mesaj[strlen(mesaj) - 1] = '\0';
    printf("[client %d] Am primit mesajul: %s\n", tdL.idThread, mesaj);

    if (strcmp(mesaj, "login") == 0)
    {
        login_user(tdL.cl);
    }
    else if (strcmp(mesaj, "register") == 0)
    {
        register_user(tdL.cl);
    }

    raspunde((struct thData *)arg);

    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    return (NULL);
};

void raspunde(void *arg)
{
    char mesaj[150], raspuns[150];
    int ok = 1;
    char category[150], product[150];
    bzero(&mesaj, sizeof(mesaj));
    bzero(&raspuns, sizeof(raspuns));

    struct thData tdL;
    tdL = *((struct thData *)arg);

    while (ok != 0)
    {
        if (read(tdL.cl, &mesaj, sizeof(mesaj)) <= 0)
        {
            printf("[client %d]\n", tdL.idThread);
            perror("Eroare la read() de la client.\n");
        }

        printf("[client %d]Mesajul a fost receptionat...%s\n", tdL.idThread, mesaj);

        if (strncmp(mesaj, "categories", 10) == 0)
        {
            categories(raspuns);
            printf("[client %d] Categorii afisate\n", tdL.idThread);
        }

        else if (strncmp(mesaj, "show products", 8) == 0)
        {
            bzero(category, sizeof(category));
            read(tdL.cl, category, sizeof(category));
            products(category, raspuns);
        }

        else if (strncmp(mesaj, "add to cart", 11) == 0)
        {

            bzero(product, sizeof(product));
            read(tdL.cl, product, sizeof(product));
            add_to_cart(product, raspuns);
        }

        else if (strncmp(mesaj, "show cart", 9) == 0)
        {
            show_cart(tdL.cl, raspuns);
            printf("Clientul vizualizeaza cosul de cumparaturi\n");
        }

        else if (strncmp(mesaj, "buy cart", 8) == 0)
        {
            buy_cart();
            printf("Comanda plasata\n");
            strcpy(raspuns, "\nComanda a fost plasata cu succes! Va mai asteptam!\n");
        }

        else if (strncmp(mesaj, "exit", 4) == 0)
        {
            exit_function();
            printf("[client %d] Client deconectat\n", tdL.idThread);
            ok = 0;
            break;
        }
        else
            strcpy(raspuns, "\nAceasta comanda nu exista!\n");

        /*pregatim mesajul de raspuns */
        printf("[client %d]Trimitem mesajul inapoi...%s", tdL.idThread, raspuns);

        /* returnam mesajul clientului */
        if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
        {
            printf("[client %d] ", tdL.idThread);
            perror("[server]Eroare la write() catre client.\n");
        }
        else
            printf("[client %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
        bzero(&mesaj, sizeof(mesaj));
        bzero(&raspuns, sizeof(raspuns));
    }
}

void categories(char *raspuns)
{
    char line[100];

    bzero(raspuns, sizeof(raspuns));

    strcat(raspuns, "\nCategoriile magazinului: ");
    FILE *products_fd = fopen("products.txt", "r");
    if (products_fd == NULL)
    {
        printf("Eroare la deschiderea fisierului.\n");
    }
    else
    {
        while (fgets(line, sizeof(line), products_fd) != NULL)
        {
            char *p = strtok(line, ":");
            strcat(raspuns, p);
            strcat(raspuns, " ");
        }
        strcat(raspuns, "\n");
    }
    fclose(products_fd);
}

void products(char *category, char *raspuns)
{
    char line[100];

    bzero(raspuns, sizeof(raspuns));
    printf("Categoria dorita este: %s\n", category);

    FILE *products_fd = fopen("products.txt", "r");
    if (products_fd == NULL)
    {
        printf("Eroare la deschiderea fisierului.\n");
    }
    else
    {
        while (fgets(line, sizeof(line), products_fd) != NULL)
        {
            char *p = strtok(line, ":");
            if (strcmp(p, category) == 0)
            {
                p = strtok(NULL, ":");
                strcpy(raspuns, "\n");
                strcat(raspuns, p);
                strcat(raspuns, "\n");
                printf("Categoria %s a fost afisata\n", category);
                break;
            }
            else
            {
                strcpy(raspuns, "\nAceasta categorie este inexistenta!\n");
                printf("Categoria cautata nu s-a gasit in fisier\n");
            }
        }
    }
    fclose(products_fd);
}

void add_to_cart(char *product, char *raspuns)
{
    char cart_name[150];
    char line[100];
    int gasit = 0;

    bzero(raspuns, sizeof(raspuns));
    bzero(cart_name, sizeof(cart_name));

    strcpy(cart_name, usernameCopy);
    strcat(cart_name, ".txt");
    FILE *products_fd = fopen("products.txt", "r");
    if (products_fd == NULL)
    {
        printf("Eroare la deschiderea fisierului cu produse.\n");
    }
    else
    {
        while (fgets(line, sizeof(line), products_fd) != NULL)
        {
            char *p = strtok(line, " ");
            while (p != NULL)
            {
                if (strcmp(p, product) == 0)
                {
                    gasit = 1;
                    FILE *cart_fd = fopen(cart_name, "a+");
                    if (cart_fd == NULL)
                    {
                        printf("Eroare la deschiderea fisierului.\n");
                    }
                    else
                    {
                        fprintf(cart_fd, "%s ", product);
                        fclose(cart_fd);
                        strcpy(raspuns, "\nProdusul a fost adaugat! Mai ai nevoie de ceva?\n");
                        printf("Am adaugat produsele in fisier.\n");
                        bzero(product, sizeof(product));
                    }
                    break;
                }
                p = strtok(NULL, " ");
            }
        }
        if (gasit == 0)
        {
            strcpy(raspuns, "\nAcest produs nu se gaseste in magazinul nostru!\n");
        }
        fclose(products_fd);
    }
}

void show_cart(int socket, char *raspuns)
{
    char line[150];
    char cart_name[150];

    bzero(raspuns, sizeof(raspuns));
    bzero(cart_name, sizeof(cart_name));

    strcpy(cart_name, usernameCopy);
    strcat(cart_name, ".txt");

    FILE *cart_fd = fopen(cart_name, "r");
    if (cart_fd == NULL)
    {
        strcpy(raspuns, "\nCosul de cumparaturi este gol. Let's do some shopping!\n");
    }
    else
    {
        while (fgets(line, sizeof(line), cart_fd) != NULL)
        {
            strcpy(raspuns, "\n");
            strcat(raspuns, line);
            strcat(raspuns, "\n");
        }
        fclose(cart_fd);
    }
}

int buy_cart()
{
    char cart_name[150];

    bzero(cart_name, sizeof(cart_name));

    strcpy(cart_name, usernameCopy);
    strcat(cart_name, ".txt");

    if (remove(cart_name) == 0)
        printf("Cos de cumparaturi golit\n");
    else
        printf("Fisierul corespunzator cosului de cumparaturi nu poate fi sters\n");
    return 0;
}

int exit_function()
{
    char cart_name[150];

    bzero(cart_name, sizeof(cart_name));

    strcpy(cart_name, usernameCopy);
    strcat(cart_name, ".txt");

    if (remove(cart_name) == 0)
        printf("Cos de cumparaturi golit\n");
    else
        printf("Fisierul corespunzator cosului de cumparaturi nu poate fi sters sau a fost sters deja\n");
    return 0;
}