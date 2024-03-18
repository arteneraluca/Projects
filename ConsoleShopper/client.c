#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
void welcome(int sd);

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
  char buf[150];             // mesajul trimis
  char raspuns[150];
  int ok = 1;
  char category[150], product[150];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  /* citirea mesajului */
  printf("[client]Introduceti comanda: (login/register)\n");
  fflush(stdout);
  bzero(&buf, sizeof(buf));
  read(0, buf, sizeof(buf));

  /* trimiterea mesajului la server */
  if (write(sd, &buf, sizeof(buf)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    return errno;
  }

  // login sau register
  welcome(sd);

  printf("Comenzi existente:\n"
         ">> categories\n"
         ">> show products\n"
         ">> add to cart\n"
         ">> show cart\n"
         ">> buy cart\n"
         ">> exit\n\n");

  while (ok != 0)
  {
    bzero(&buf, sizeof(buf));
    bzero(&raspuns, sizeof(raspuns));

    printf("[client] Introduceti comanda: ");

    fflush(stdout);
    read(0, buf, sizeof(buf));

    /* trimiterea mesajului la server */
    if (write(sd, &buf, sizeof(buf)) <= 0)
    {
      perror("[client] Eroare la write() spre server.\n");
      return errno;
    }

    else if (strncmp(buf, "show products", 8) == 0)
    {
      printf("[client] Introduceti categoria de produse pe care doriti sa le vizualizati: ");
      scanf("%s", category);
      write(sd, &category, sizeof(category));
    }

    else if (strncmp(buf, "add to cart", 11) == 0)
    {
      printf("[client] Introduceti produsul dorit: ");
      scanf("%s", product);
      write(sd, &product, sizeof(product));
    }

    else if (strncmp(buf, "exit", 4) == 0)
    {
      close(sd);
      ok = 0;
      break;
    }

    /* citirea raspunsului dat de server
   (apel blocant pina cind serverul raspunde) */
    if (read(sd, &raspuns, sizeof(raspuns)) < 0)
    {
      perror("[client] Eroare la read() de la server.\n");
      return errno;
    }
    /* afisam mesajul primit */
    printf("%s\n", raspuns);
    fflush(stdout);
    bzero(&buf, sizeof(buf));
  }

  /* inchidem conexiunea, am terminat */
  close(sd);
}

void welcome(int sd)
{
  printf("\n--Welcome to ConsoleShopper--\n");
  char mesaj[150], raspuns[150];
  int ok = 0;

  while (ok == 0)
  {
    bzero(&mesaj, sizeof(mesaj));
    bzero(&raspuns, sizeof(raspuns));

    if (read(sd, mesaj, 150) < 0)
    {
      perror("[client] Eroare la read() de la server.\n");
    }

    printf("%s\n", mesaj);

    if (strcmp(mesaj, "\n--Autentificat cu succes--\n") != 0) // inca nu a reusit sa se conecteze
    {
      // citeste de la tastatura
      fflush(stdout);
      read(0, raspuns, sizeof(raspuns));
      // trimiterea mesajului la server
      if (write(sd, raspuns, 30) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
      }
    }
    else
    {
      break;
      ok = 1;
    }
  }
}