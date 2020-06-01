---
title: "CMessenger"
contributors: "HALLGREN Matthias & MASTRANGELO Julien"
---

- [CMessenger](#cmessenger)
    - [Setup](#setup)
    - [Objectif du projet](#objectif-du-projet)
    - [Fonctionnalités](#fonctionnalités)
    - [Organisation de l'application](#organisation-de-lapplication)
    - [Crédits](#crédits)

# CMessenger


### Setup

---

1. Cloner le projet :

```
$ git clone https://github.com/EvolveArt/CMessenger.git
```

2. Compiler la librairie :

```
$ cd modules
$ make
```

3. Compiler les programmes Client et Serveur :

```
$ make
```

4. Lancer le serveur :

```
$ cd Server
$ ./serveur 5000
```

5. Lancer un client :

```
$ cd Client
$ ./client localhost 5000 matthias
```

6. Lancer autant de clients que souhaité :

```
Répéter l'étape n°5
```

### Objectif du projet

---

Avec ce projet, le but est d'appliquer nos connaissances en programmation système. Nous avons décider de réaliser un système de Chat Room, similaire à ce que fait Messenger la messagerie de Facebook. Nous utilisons pour celà une architecture statique de Client/Serveur *multithread*.
Le client et le serveur communiquent avec des *sockets* TCP/IP.

### Fonctionnalités

---

| Fonctionnalité                                 | Statut |
| ---------------------------------------------- | ------ |
| Création d'une nouvelle Chat Room              | ✅      |
| Rejoindre une Chat Room existante              | ✅      |
| Le pseudo choisi est unique                    | ✅      |
| Quitter une Chat Room                          | ❌      |
| Voir les personnes présentes dans la Chat Room | ❌      |
| Envoyer des messages privés                    | ❌      |
| Système d'authentification                     | ❌      |
| Log des messages dans `journal.log`            | ✅      |


### Organisation de l'application

---

- `./include/chatroom.h`: Structures, Enumérations, Prototypes utilisées spécifiquement pour le système de Chat Room.
  </br>
- `./modules/chatroom.c`: Fonctions utiles à la gestion de la liste chaînée de Chat Rooms. 
  </br>
- `./Server/serveur.c`: Le Serveur s'occupe d'accepter les connexions des clients et de les assigner à un thread *worker* précis. Il gère également la liste des Chat Rooms et performe les actions souhaitées par les clients.
  </br>
- `./Client/client.c`: Le Client comporte deux threads. </br>Le thread principal dans lequel il envoie des données ou des actions au Serveur. </br>Un thread dans lequel il lit les messages des autres clients que lui envoie le serveur et les affiche.

### Crédits

***

Les deux seuls contributeurs de ce projet sont :
*Hallgren Matthias & Mastrangelo Julien*

Merci surtout à notre professeur M. Pierre UNY pour ses cours sans lequels ce projet n'aurait pas pu exister.


