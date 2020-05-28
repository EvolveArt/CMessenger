// client.c
#include "pse.h"

#define CMD "client"

ChatRoom *mainMenu();
void str_trim_lf(char *arr, int length);
static void clearStdin();

int main(int argc, char *argv[])
{
  int sock, ret;
  struct sockaddr_in *adrServ;
  int fin = FAUX;
  char ligne[LIGNE_MAX];

  ChatRoom *currentChatroom = NULL;

  signal(SIGPIPE, SIG_IGN);

  if (argc != 3)
    erreur("usage: %s machine port\n", argv[0]);

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

  currentChatroom = mainMenu();

  system("clear");
  printf("--- ChatRoom n°%d : %s --- \n", currentChatroom->room_id, currentChatroom->name);

  while (!fin)
  {
    printf("ligne> ");
    if (fgets(ligne, LIGNE_MAX, stdin) == NULL)
      // sortie par CTRL-D
      fin = VRAI;
    else
    {
      if (ecrireLigne(sock, ligne) == -1)
        erreur_IO("ecriture socket");

      if (strcmp(ligne, "fin\n") == 0)
        fin = VRAI;
    }
  }

  if (close(sock) == -1)
    erreur_IO("fermeture socket");

  exit(EXIT_SUCCESS);
}

ChatRoom *mainMenu()
{
  char room_name[MAX_ROOM_NAME];
  ChatRoom *room = (ChatRoom *)malloc(sizeof(ChatRoom));

  int choice = 0;
  int room_choice = -1;

  while (VRAI)
  {
    // system("clear");

    printf("1. Créer une nouvelle ChatRoom\n");
    printf("2. Rejoindre une ChatRoom\n");

    choice = getchar();

    switch (choice)
    {
    case '1':
      printf("Donnez un nom à la room : ");
      clearStdin();
      fgets(room_name, MAX_ROOM_NAME, stdin);
      str_trim_lf(room_name, strlen(room_name));

      room = addNewChatRoom(room_name, 1);
      joinChatRoom(room->room_id);

      return room;
      break;

    case '2':
      if (chatroomsList == NULL)
      {
        printf("Il n'existe actuellement aucune chatroom. Créez en une !\n");
        break;
      }
      printChatRoomList();
      printf("Quelle room voulez vous rejoindre ? (id) ");
      scanf("%d", &room_choice);
      room = joinChatRoom(room_choice);
      return room;
      break;

    default:
      printf("Choix inconnu. Veuillez réessayer.\n");
      break;
    }
  }
}

void str_trim_lf(char *arr, int length)
{
  int i;
  for (i = 0; i < length; i++)
  { // trim \n
    if (arr[i] == '\n')
    {
      arr[i] = '\0';
      break;
    }
  }
}

static void clearStdin()
{
  int c;
  while ((c = getchar()) != EOF && c != '\n')
  {
  }
}
