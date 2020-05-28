// client.c
#include "pse.h"

#define CMD   "client"

void sendUsername(int fd, char* username);

int main(int argc, char *argv[]) {
  int sock, ret;
  struct sockaddr_in *adrServ;
  int fin = FAUX;
  char ligne[LIGNE_MAX];
  char username[LIGNE_MAX];

  signal(SIGPIPE, SIG_IGN);

  if (argc != 4)
    erreur("usage: %s machine port username\n", argv[0]);

  strcpy(username, argv[3]);

  printf("%s: creating a socket\n", CMD);
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    erreur_IO("socket");

  printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);
  adrServ = resolv(argv[1], argv[2]);
  if (adrServ == NULL)
    erreur("adresse %s port %s inconnus\n", argv[1], argv[2]);

  printf("%s: adr %s, port %hu\n", CMD,
	        stringIP(ntohl(adrServ->sin_addr.s_addr)),
	        ntohs(adrServ->sin_port));

  printf("%s: connecting the socket\n", CMD);
  ret = connect(sock, (struct sockaddr *)adrServ, sizeof(struct sockaddr_in));
  if (ret < 0)
    erreur_IO("connect");

  sendUsername(sock, username);
  if(strncmp(username, argv[3], LIGNE_MAX) != 0)
  {
    printf("%s already used.\n", argv[3]);
  }
  else
  {
    while (!fin)
    {
      printf("ligne> ");
      if (fgets(ligne, LIGNE_MAX, stdin) == NULL)
        // sortie par CTRL-D
        fin = VRAI;
      else {
        if (ecrireLigne(sock, ligne) == -1)
          erreur_IO("ecriture socket");

        if (strcmp(ligne, "fin\n") == 0)
          fin = VRAI;
      }
    }
  }

  if (close(sock) == -1)
    erreur_IO("fermeture socket");

  exit(EXIT_SUCCESS);
}

void sendUsername(int fd, char* username)
{
  if (write(fd, username, sizeof(username)) == -1)
    erreur_IO("ecriture socket");

  if (read(fd, username, sizeof(username)) == -1)
    erreur_IO("lecture socket");
}