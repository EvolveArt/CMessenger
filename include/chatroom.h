#ifndef CHATROOM_INCLUDE_H
#define CHATROOM_INCLUDE_H

#define MAX_ROOM_NAME 50

/**
* Structure for a chat room
* Used to handle the state of the room
*/
typedef struct ChatRoom_t
{
    int room_id;              // unique id of the room
    int worker_id;            // worker thread for the chat room
    char name[MAX_ROOM_NAME]; // name of the room

    int nbr_clients; // number of clients in the room
} ChatRoom;

/**
* Structure for a chat room
* Used to handle the state of the room
*/
typedef struct ChatRooms_t
{
    ChatRoom chatroom;
    struct ChatRooms_t *next; // next chatroom in the linked list if any
} ChatRooms;

/* Linked list head of ChatRooms */
extern ChatRooms *chatroomsList;

/* Init the chatrooms list */
void initChatRooms(void);

/* Create a chat room , add it to the list and return it*/
ChatRoom *addNewChatRoom(char room_name[MAX_ROOM_NAME], int worker_id);

/* Free and remove from the list a chat room given its id */
void freeChatRoom(int room_id);

/* Join a chat room */
ChatRoom *joinChatRoom(int room_id);

/* Display the list of chatrooms */
void printChatRoomList();

#endif