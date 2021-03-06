// serveur.c
#include "pse.h"

#define CMD "serveur"
#define NOM_JOURNAL "journal.log"

#define NB_WORKERS 9

void creerCohorteWorkers(void); //initialise les threads (workers) et les sémaphores associés
int chercherWorkerLibre(void);
void *threadWorker(void *arg);
int checkUsername(int canal, char *username);                          // check if this client's username is already connected
int checkPassword(int canal, char *username, unsigned char *password); // check if the client's password fits with the username
int executeUserAction(DataSpec *_dataSpec);                            // so the client can create/join a room
void sessionClient(int canal, char *username, int room_id);
int ecrireDansJournal(char *ligne);
void remiseAZeroJournal(void);
void lockMutexFdJournal(void);
void unlockMutexFdJournal(void);
void lockMutexCanal(int numWorker);
void unlockMutexCanal(int numWorker);
void lockMutexUsername(int numWorker);
void unlockMutexUsername(int numWorker);
void lockMutexRoomID(int numWorker);
void unlockMutexRoomID(int numWorker);

void serializeChatroom(int _canal, ChatRoom *source);

int fdJournal;
DataSpec dataSpec[NB_WORKERS];
sem_t semWorkersLibres;

// mutex pour acces concurrent au descripteur du fichier journal et au canal de
// chaque worker
pthread_mutex_t mutexFdJournal;
pthread_mutex_t mutexCanal[NB_WORKERS];
pthread_mutex_t mutexUsername[NB_WORKERS];
pthread_mutex_t mutexRoomID[NB_WORKERS];

int main(int argc, char *argv[])
{
  short port;
  int ecoute, canal, ret;
  struct sockaddr_in adrEcoute, adrClient;
  unsigned int lgAdrClient;
  int numWorkerLibre;

  char *globalChatroom = "CMessenger Global";

  initChatRooms();
  addNewChatRoom(globalChatroom);

  if (argc != 2)
    erreur("usage: %s port\n", argv[0]);

  port = (short)atoi(argv[1]);

  fdJournal = open(NOM_JOURNAL, O_CREAT | O_WRONLY | O_APPEND, 0644);
  if (fdJournal == -1)
    erreur_IO("ouverture journal");

  creerCohorteWorkers();

  ret = sem_init(&semWorkersLibres, 0, NB_WORKERS);
  if (ret == -1)
    erreur_IO("init semaphore workers libres");

  printf("%s: creating a socket\n", CMD);
  ecoute = socket(AF_INET, SOCK_STREAM, 0);
  if (ecoute < 0)
    erreur_IO("socket");

  adrEcoute.sin_family = AF_INET;
  adrEcoute.sin_addr.s_addr = INADDR_ANY;
  adrEcoute.sin_port = htons(port);
  printf("%s: binding to INADDR_ANY address on port %d\n", CMD, port);
  ret = bind(ecoute, (struct sockaddr *)&adrEcoute, sizeof(adrEcoute));
  if (ret < 0)
    erreur_IO("bind");

  printf("%s: listening to socket\n", CMD);
  ret = listen(ecoute, 5);
  if (ret < 0)
    erreur_IO("listen");

  while (VRAI)
  {
    printf("%s: accepting a connection\n", CMD);
    lgAdrClient = sizeof(adrClient);
    canal = accept(ecoute, (struct sockaddr *)&adrClient, &lgAdrClient);
    if (canal < 0)
      erreur_IO("accept");

    printf("%s: adr %s, port %hu\n", CMD,
           stringIP(ntohl(adrClient.sin_addr.s_addr)), ntohs(adrClient.sin_port));

    ret = sem_wait(&semWorkersLibres);
    if (ret == -1)
      erreur_IO("wait semaphore workers libres");
    numWorkerLibre = chercherWorkerLibre();

    dataSpec[numWorkerLibre].canal = canal;
    ret = sem_post(&dataSpec[numWorkerLibre].sem);
    if (ret == -1)
      erreur_IO("post semaphore worker");

    sleep(1);
  }

  if (close(ecoute) == -1)
    erreur_IO("fermeture ecoute");

  if (close(fdJournal) == -1)
    erreur_IO("fermeture journal");

  exit(EXIT_SUCCESS);
}

void creerCohorteWorkers(void)
{
  int i, ret;

  for (i = 0; i < NB_WORKERS; i++)
  {
    dataSpec[i].canal = -1;
    dataSpec[i].tid = i;
    strcpy(dataSpec[i].username, "");

    ret = sem_init(&dataSpec[i].sem, 0, 0);
    if (ret == -1)
      erreur_IO("init semaphore worker");

    ret = pthread_create(&dataSpec[i].id, NULL, threadWorker, &dataSpec[i]);
    if (ret != 0)
      erreur_IO("creation worker");
  }
}

// retourne le no. du worker ou -1 si pas de worker libre
int chercherWorkerLibre(void)
{
  int i, canal;

  for (i = 0; i < NB_WORKERS; i++)
  {
    lockMutexCanal(i);
    canal = dataSpec[i].canal;
    unlockMutexCanal(i);
    if (canal < 0)
      return i;
  }

  return -1;
}

void *threadWorker(void *arg)
{
  DataSpec *dataSpec = (DataSpec *)arg;
  int ret;
  char username[LIGNE_MAX];
  unsigned char password[LIGNE_MAX];

  while (VRAI)
  {
    ret = sem_wait(&dataSpec->sem);
    if (ret == -1)
      erreur_IO("wait semaphore worker");

    printf("worker %d: reveil\n", dataSpec->tid);

    lockMutexUsername(dataSpec->tid);
    if (checkUsername(dataSpec->canal, username) == 1 && checkPassword(dataSpec->canal, username, password) == 1)
    { // Si les identifiants sont corrects, on connecte le client
      strcpy(dataSpec->username, username);
      printf("serveur : user %s added\n", dataSpec->username);

      int ret;

      // Attente des actions utilisateurs (menu principal)
      while (1)
      {
        ret = executeUserAction(dataSpec);
        // printf("user action returned : %d\n", ret);
        if (ret == 1 || ret == 2)
          break;

        sleep(1);
      }

      if (ret == 1)
        sessionClient(dataSpec->canal, dataSpec->username, dataSpec->room_id);

      strcpy(dataSpec->username, ""); //libération du username une fois que le client est parti
    }
    else
    {
      printf("%s : already used\n", username);
      strcpy(dataSpec->username, "");
    }
    unlockMutexUsername(dataSpec->tid);

    lockMutexCanal(dataSpec->tid);
    dataSpec->canal = -1;
    unlockMutexCanal(dataSpec->tid);

    printf("worker %d: sommeil\n", dataSpec->tid);

    ret = sem_post(&semWorkersLibres);
    if (ret == -1)
      erreur_IO("post semaphore workers libres");
  }

  pthread_exit(NULL);
}

/**
 * Vérification du pseudo du client
 * @return 0 si déjà utilisé, 1 si OK
 * */
int checkUsername(int canal, char *username)
{
  int lgLue;

  lgLue = read(canal, username, sizeof(username));
  if (lgLue == -1)
    erreur_IO("lecture canal");

  for (int i = 0; i < NB_WORKERS; ++i)
  {
    if (strncmp(dataSpec[i].username, username, LIGNE_MAX) == 0)
    {
      char *message = "username_already_used";
      if (write(canal, message, sizeof(message)) == -1)
        erreur_IO("ecriture canal"); //envoyer au client: username déjà utilisé
      if (close(canal) == -1)
        erreur_IO("fermeture canal");
      return 0;
    }
  }
  if (write(canal, username, sizeof(username)) == -1)
    erreur_IO("ecriture canal");
  return 1;
}

/* return 1 if password given match or if it is a new user (creation of an account), otherwise 0*/
int checkPassword(int canal, char *username, unsigned char *password)
{
  if (read(canal, password, sizeof(password)) == -1)
    erreur_IO("lecture canal");
  FILE *fpasswords;
  fpasswords = fopen("passwords.txt", "r");
  if (fpasswords == NULL)
    erreur_IO("ouverture fichier passwords.txt");
  char scanUsername[LIGNE_MAX];
  char scanHashPassword[LIGNE_MAX];

  unsigned char *hashPassword = SHA256(password, LIGNE_MAX, 0);
  char sHashPassword[HASH_HEX_SIZE];
  hashToString(sHashPassword, hashPassword);

  int flag = 0;
  while (!flag && (fscanf(fpasswords, "%s %s", scanUsername, scanHashPassword) == 2))
  {
    if (strncmp(scanUsername, username, LIGNE_MAX) == 0)
    {
      flag = 1;
    }
  }
  fclose(fpasswords);
  if (flag)
  {

    if (strncmp(sHashPassword, scanHashPassword, LIGNE_MAX) == 0)
    {
      write(canal, password, sizeof(password));
      return 1;
    }
    char *message = "incorrect_password";
    write(canal, message, sizeof(message));
    /*for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
      printf("%02x", hashAttempt[i]);
    putchar('\n'); */
  }
  else // creation of an account (new user)
  {
    printf("%s : Creation of %s's account\n", CMD, username);
    fpasswords = fopen("passwords.txt", "a");
    fprintf(fpasswords, "%s %s", username, sHashPassword);
    fprintf(fpasswords, "\n");
    fclose(fpasswords);
    write(canal, password, sizeof(password));
    return 1;
  }

  return 0;
}

/**
 * Récéption et éxécution d'une action client
 * @params _dataSpec : <struct DataSpec> session client
 * @return 
 *  - 0 : l'action ne permet pas de quitter le menu
 *  - 1 : l'action permet de démarrer la session client
 *  - 2 : l'action permet de quitter le menu sans lancer de session client
 * */
int executeUserAction(DataSpec *_dataSpec)
{

  ACTION userAction;
  if (read(_dataSpec->canal, &userAction, sizeof(ACTION)) == -1)
    erreur_IO("lecture canal user action");
  if (userAction == LEAVE) // 16 Ctrl+C (le client a quitté le programme)
    return 2;
  printf("%s: Exécution d'une User Action (%d)\n", CMD, userAction);

  ChatRoom *chatroom = (ChatRoom *)malloc(sizeof(ChatRoom));

  if (userAction == CREATE)
  {
    printf("%s: Création d'une nouvelle ChatRoom\n", CMD);

    char name[MAX_ROOM_NAME];
    if (read(_dataSpec->canal, name, sizeof(name)) == -1)
      erreur_IO("lecture canal room name");

    chatroom = addNewChatRoom(name);

    // if (write(_dataSpec->canal, chatroom, sizeof(chatroom)) == -1)
    // erreur_IO("ecriture canal server chatroom");

    serializeChatroom(_dataSpec->canal, chatroom);

    return 0;
  }

  else if (userAction == JOIN)
  {
    printf("%s: Rejoindre une ChatRoom\n", CMD);

    int room_id;

    if (read(_dataSpec->canal, &room_id, sizeof(room_id)) == -1)
      erreur_IO("lecture canal");

    chatroom = joinChatRoom(room_id);

    // if (write(_dataSpec->canal, chatroom, sizeof(chatroom)) == -1)
    // erreur_IO("ecriture canal");

    serializeChatroom(_dataSpec->canal, chatroom);

    if (chatroom->room_id == -1)
      return 0;

    lockMutexRoomID(_dataSpec->tid);
    _dataSpec->room_id = chatroom->room_id;
    unlockMutexRoomID(_dataSpec->tid);

    return 1;
  }

  else if (userAction == DISPLAY)
  {
    printf("%s: Affichage de la liste des ChatRooms\n", CMD);
    printChatRoomList(_dataSpec->canal);

    //if (write(_dataSpec->canal, chatroomsList, sizeof(chatroomsList)) == -1)
    //erreur_IO("ecriture canal");

    return 0;
  }

  char *message = "Action inconnue.";
  if (write(_dataSpec->canal, message, sizeof(message)) == -1)
    erreur_IO("ecriture canal");

  return 0;
}

/**
 * Fonction utilisée pour la sérialisation d'un object de type <struct ChatRoom>
 * @params :
 *  - _canal : socket de connection avec le client
 *  - source : <struct ChatRoom> à envoyer sur le canal _canal
 * */
void serializeChatroom(int _canal, ChatRoom *source)
{

  if (write(_canal, source->name, sizeof(source->name)) == -1)
    erreur_IO("ecriture canal");

  if (write(_canal, &(source->room_id), sizeof(source->room_id)) == -1)
    erreur_IO("ecriture canal");

  if (write(_canal, &(source->nbr_clients), sizeof(source->nbr_clients)) == -1)
    erreur_IO("ecriture canal");
}

void sessionClient(int canal, char *username, int room_id)
{
  int fin = FAUX;

  char ligne[LIGNE_MAX];
  char message[LIGNE_MAX] = "";
  int lgLue, lgEcr;

  char private_message[LIGNE_MAX] = "";

  while (!fin)
  {
    lgLue = lireLigne(canal, ligne);
    if (lgLue == -1)
      erreur_IO("lecture canal");

    else if (lgLue == 0)
    { // arret du client (CTRL-D, interruption)
      fin = VRAI;
      printf("%s: arret de %s\n", CMD, username);
    }
    else
    { // lgLue > 0
      if (strncmp(ligne, "/fin", LIGNE_MAX) == 0)
      {
        fin = VRAI;
        printf("%s: fin session %s\n", CMD, username);
      }
      else if (strncmp(ligne, "/list", LIGNE_MAX) == 0)
      {
        for (int i = 0; i < NB_WORKERS; ++i)
        {
          if (dataSpec[i].canal != -1 && strncmp(dataSpec[i].username, username, LIGNE_MAX) == 0)
          {
            executeUserAction(&dataSpec[i]);
          }
        }
      }
      else if (strncmp(ligne, "/init", LIGNE_MAX) == 0)
      {
        remiseAZeroJournal();
        printf("%s: remise a zero du journal par %s\n", CMD, username);
      }
      else if (strncmp(ligne, "/msg", 4) == 0)
      { // Envoi de messages privés
        char *receiver;
        char *buffer;

        strtok(ligne, " "); // Get rid of the command (/msg)
        receiver = strtok(NULL, " ");
        buffer = strtok(NULL, "\n");

        if (!receiver || !buffer)
          continue;

        printf("%s: sending '%s' to %s from %s\n", CMD, buffer, receiver, username);

        memset(private_message, '\0', LIGNE_MAX);
        strcpy(private_message, "[private] ");
        strcat(private_message, username);
        strcat(private_message, "> ");
        strcat(private_message, buffer);

        for (int i = 0; i < NB_WORKERS; ++i)
        {
          if (dataSpec[i].canal != -1 && dataSpec[i].username != username && dataSpec[i].room_id == room_id && strcmp(dataSpec[i].username, receiver) == 0)
          {
            // Envoi du message privé au client de pseudo == receiver
            if (ecrireLigne(dataSpec[i].canal, private_message) < 0)
              erreur_IO("ecriture canal");
          }
        }
      }
      else
      {
        // Ecriture d'un message dans la room du client

        char roomIndicator[MAX_ROOM_NAME + 5];
        sprintf(roomIndicator, "[%s] ", getChatRoomByID(room_id)->name);

        memset(message, '\0', LIGNE_MAX);
        strcat(message, roomIndicator);
        strcat(message, username);
        strcat(message, "> ");
        strcat(message, ligne);

        printf("%s\n", message);
        lgEcr = ecrireLigne(fdJournal, message);
        if (lgEcr < 0)
          erreur_IO("ecriture journal");
        printf("%s: ligne de %d octets ecrite dans journal par %s\n", CMD, lgEcr, username);

        for (int i = 0; i < NB_WORKERS; ++i)
        {
          if (dataSpec[i].canal != -1 && dataSpec[i].username != username && dataSpec[i].room_id == room_id)
          {
            if (ecrireLigne(dataSpec[i].canal, message) < 0)
              erreur_IO("ecriture canal");
          }
        }
      }
    }
  }

  // On actualise le nombre de clients de la room lorsque le client la quitte
  getChatRoomByID(room_id)->nbr_clients--;

  if (close(canal) == -1)
    erreur_IO("fermeture canal");
}

int ecrireDansJournal(char *ligne)
{
  int lg;

  lockMutexFdJournal();
  lg = ecrireLigne(fdJournal, ligne);
  unlockMutexFdJournal();

  return lg;
}

void remiseAZeroJournal(void)
{
  lockMutexFdJournal();

  if (close(fdJournal) < 0)
    erreur_IO("fermeture journal pour remise a zero");

  fdJournal = open("journal.log", O_TRUNC | O_WRONLY | O_APPEND);
  if (fdJournal < 0)
    erreur_IO("reouverture journal");

  unlockMutexFdJournal();
}

void lockMutexFdJournal(void)
{
  int ret;

  ret = pthread_mutex_lock(&mutexFdJournal);
  if (ret != 0)
    erreur_IO("lock mutex descipteur journal");
}

void unlockMutexFdJournal(void)
{
  int ret;

  ret = pthread_mutex_unlock(&mutexFdJournal);
  if (ret != 0)
    erreur_IO("unlock mutex descipteur journal");
}

void lockMutexCanal(int numWorker)
{
  int ret;

  ret = pthread_mutex_lock(&mutexCanal[numWorker]);
  if (ret != 0)
    erreur_IO("lock mutex canal");
}

void unlockMutexCanal(int numWorker)
{
  int ret;

  ret = pthread_mutex_unlock(&mutexCanal[numWorker]);
  if (ret != 0)
    erreur_IO("unlock mutex canal");
}

void lockMutexUsername(int numWorker)
{
  int ret;

  ret = pthread_mutex_lock(&mutexUsername[numWorker]);
  if (ret != 0)
    erreur_IO("lock mutex username");
}

void unlockMutexUsername(int numWorker)
{
  int ret;

  ret = pthread_mutex_unlock(&mutexUsername[numWorker]);
  if (ret != 0)
    erreur_IO("unlock mutex username");
}

void lockMutexRoomID(int numWorker)
{
  int ret;

  ret = pthread_mutex_lock(&mutexRoomID[numWorker]);
  if (ret != 0)
    erreur_IO("lock mutex roomID");
}

void unlockMutexRoomID(int numWorker)
{
  int ret;

  ret = pthread_mutex_unlock(&mutexRoomID[numWorker]);
  if (ret != 0)
    erreur_IO("unlock mutex roomID");
}