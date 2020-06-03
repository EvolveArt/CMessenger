#include "pse.h"

ChatRooms *chatroomsList;

void initChatRooms(void)
{
    chatroomsList = NULL;
}

ChatRoom *addNewChatRoom(char *room_name)
{
    ChatRoom *_chatroom = (ChatRoom *)malloc(sizeof(ChatRoom));

    strcpy(_chatroom->name, room_name);
    // _chatroom->worker_id = worker_id;

    if (chatroomsList == NULL)
    {
        _chatroom->room_id = 0;
        _chatroom->nbr_clients = 0;

        ChatRooms *_new = (ChatRooms *)malloc(sizeof(ChatRooms));
        _new->chatroom = *_chatroom;
        _new->next = NULL;

        chatroomsList = _new;
    }
    else
    {
        // Get the last chatroom
        ChatRooms *cur = chatroomsList;
        while (cur->next != NULL)
        {
            cur = cur->next;
        }

        _chatroom->room_id = cur->chatroom.room_id + 1;
        _chatroom->nbr_clients = 0;

        ChatRooms *_new = (ChatRooms *)malloc(sizeof(ChatRooms));
        _new->chatroom = *_chatroom;
        _new->next = NULL;

        cur->next = _new;
    }

    return _chatroom;
}

void freeChatRoom(int room_id)
{
    ChatRooms *cur = chatroomsList;
    ChatRooms *prev = NULL;

    // Find the room with the given id
    while (cur->next)
    {
        if (cur->chatroom.room_id == room_id)
        {
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    if (cur->chatroom.room_id != room_id)
    {
        printf("No room found with the id : room_id = %d", room_id);
        return;
    }

    // Remove it from the linked list
    prev->next = cur->next;
    free(cur);
}

ChatRoom *joinChatRoom(int room_id)
{
    ChatRoom *actualChatroom = getChatRoomByID(room_id);

    actualChatroom->nbr_clients++;

    return actualChatroom;
}

void printChatRoomList(int canal)
{
    ChatRooms *cur = chatroomsList;

    while (cur)
    {
        char ligne[LIGNE_MAX] = {0};
        snprintf(ligne, LIGNE_MAX - 1, "(%d clients) Room %d : %s\n", cur->chatroom.nbr_clients, cur->chatroom.room_id, cur->chatroom.name);
        printf("ligne : %s", ligne);

        if (write(canal, ligne, sizeof(ligne)) == -1)
            erreur_IO("ecriture canal");

        cur = cur->next;
    }

    char end_message[LIGNE_MAX] = {0};
    strcpy(end_message, "end_list");
    if (write(canal, end_message, sizeof(end_message)) == -1)
        erreur_IO("écriture canal");
}

ChatRoom *getChatRoomByID(int room_id)
{
    ChatRooms *cur = chatroomsList;

    // Find the room with the given id
    while (cur->next)
    {
        if (cur->chatroom.room_id == room_id)
        {
            break;
        }
        cur = cur->next;
    }

    if (cur->chatroom.room_id != room_id)
    {
        printf("Pas de room trouvée avec 'room_id' = %d\n", room_id);

        ChatRoom *error_room = (ChatRoom *)malloc(sizeof(ChatRoom));
        strcpy(error_room->name, "");
        error_room->nbr_clients = 0;
        error_room->room_id = -1;

        return error_room;
    }

    return &cur->chatroom;
}
