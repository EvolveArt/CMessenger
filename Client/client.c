// client.c
#include "pse.h"

#define CMD "client"

void sendUsername(int fd, char *username);
void sendUserAction(int fd, ACTION action, void *args);

void *receiveMessage(void *arg);
void mainMenu();
void str_trim_lf(char *arr, int length);
static void clearStdin();

int sock;
ChatRoom *currentChatroom;
ChatRooms *_chatroomsList;

int main(int argc, char *argv[])
{
  int ret;
  struct sockaddr_in *adrServ;
  int fin = FAUX;
  char ligne[LIGNE_MAX];
  char username[LIGNE_MAX];
  pthread_t idThreadReceive; // thread qui gère la réception de messages des autres clients

  currentChatroom = (ChatRoom *)malloc(sizeof(ChatRoom));
  _chatroomsList = NULL;

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
  if (strncmp(username, argv[3], LIGNE_MAX) != 0)
  {
    printf("%s already used.\n", argv[3]);
  }
  else
  {
    mainMenu();

    system("clear");
    printf("--- ChatRoom n°%d : %s --- \n", currentChatroom->room_id, currentChatroom->name);

    // réception des messages
    pthread_create(&idThreadReceive, NULL, receiveMessage, NULL);
    //envoi des messages
    while (!fin)
    {
      printf("> ");
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
  }

  if (close(sock) == -1)
    erreur_IO("fermeture socket");

  exit(EXIT_SUCCESS);
}

void mainMenu()
{
  char room_name[MAX_ROOM_NAME];

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
      printf("Donnez un nom à la room :  ");
      clearStdin();
      fgets(room_name, MAX_ROOM_NAME, stdin);
      str_trim_lf(room_name, strlen(room_name));

      sendUserAction(sock, CREATE, room_name);

      break;

    case '2':
      sendUserAction(sock, DISPLAY, NULL);

      printf("Quelle room voulez vous rejoindre ? (id) ");
      scanf("%d", &room_choice);

      sendUserAction(sock, JOIN, &room_choice);

      if (currentChatroom->room_id != -1)
        return;

      printf("ID non valide, veuillez réessayer.\n");

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

void sendUsername(int fd, char *username)
{
  if (write(fd, username, sizeof(username)) == -1)
    erreur_IO("ecriture socket");

  if (read(fd, username, sizeof(username)) == -1)
    erreur_IO("lecture socket");
}

void sendUserAction(int fd, ACTION action, void *args)
{
  printf("%s: User Action %d \n", CMD, action);

  if (write(fd, &action, sizeof(action)) == -1)
    erreur_IO("ecriture socket");

  if (args != NULL)
  {
    printf("%s: Envoi de données\n", CMD);

    if (write(fd, args, sizeof(args)) == -1)
      erreur_IO("ecriture socket");
  }

  if (action == DISPLAY)
  {
    printf("Voici la liste des Chat Rooms : \n");

    char room_name[LIGNE_MAX];

    while (1)
    {

      if (read(fd, room_name, sizeof(room_name)) == -1)
        erreur_IO("lecture socket DISPLAY");

      if (strstr(room_name, "end_list") != NULL)
        break;

      printf("%s", room_name);
    }
    //if (read(fd, _chatroomsList, sizeof(_chatroomsList)) == -1)
    //erreur_IO("lecture socket DISPLAY");
  }
  else
  {
    if (read(fd, currentChatroom, sizeof(currentChatroom)) == -1)
      erreur_IO("lecture socket ACTION");
  }
}

void *receiveMessage(void *arg)
{
  char ligne[LIGNE_MAX];

  while (1)
  {
    if (lireLigne(sock, ligne) == -1)
      erreur_IO("lecture socket");
    printf("%s\n", ligne);
  }
}
