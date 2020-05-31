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
        erreur("No room found with the id : room_id = %d", room_id);

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

void printChatRoomList()
{
    ChatRooms *cur = chatroomsList;

    while (cur)
    {
        printf("(%d clients) Room %d : %s\n", cur->chatroom.nbr_clients, cur->chatroom.room_id, cur->chatroom.name);
        cur = cur->next;
    }
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
        erreur("No room found with the id : room_id = %d", room_id);

    return &cur->chatroom;
}
