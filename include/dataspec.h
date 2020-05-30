#ifndef DATASPEC_INCLUDE_H
#define DATASPEC_INCLUDE_H

/* module datathread : donnees specifiques */

/* donnees specifiques */
typedef struct DataSpec_t
{
  pthread_t id; /* identifiant du thread */
  int libre;    /* indicateur de terminaison */
                /* ajouter donnees specifiques après cette ligne */
  int tid;      /* identifiant logique */
  int canal;    /* canal de communication */
  int room_id;  /* chatroom liée a ce thread */
  sem_t sem;    /* semaphore de reveil */
  char username[LIGNE_MAX];
} DataSpec;

#endif
