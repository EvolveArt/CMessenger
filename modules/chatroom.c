#include "pse.h"

ChatRooms *chatroomsList;

ChatRoom *addNewChatRoom(char room_name[MAX_ROOM_NAME], int worker_id) {
    ChatRoom *_chatroom = (ChatRoom *) malloc(sizeof(ChatRoom));
    strcpy(_chatroom->name, room_name);
    _chatroom->worker_id = worker_id;
    
    // Get the last chatroom
    ChatRooms *cur = chatroomsList;
    while(cur->next) {
        cur = chatroomsList->next;
    }

    _chatroom->room_id = chatroomsList ? cur->chatroom.room_id + 1 : 0;
    _chatroom->nbr_clients = 0;

    // Append the new chatroom to the list
    ChatRooms *_new = (ChatRooms *) malloc(sizeof(ChatRooms));
    _new->chatroom = *_chatroom;
    _new->next = NULL;
    
    // In case it's the first room to be created
    if(!chatroomsList) {
        chatroomsList = _new;
    } else {
        cur->next = _new;
    }

    return _chatroom;
}

void freeChatRoom(int room_id) {
    ChatRooms *cur = chatroomsList;
    ChatRooms *prev = NULL;

    // Find the room with the given id
    while(cur->next) {
        if(cur->chatroom.room_id == room_id) {
            break;
        }
        prev = cur;
        cur = chatroomsList->next;
    }

    if(cur->chatroom.room_id != room_id)
        erreur("No room found with the id : room_id = %d", room_id);

    // Remove it from the linked list
    prev->next = cur->next;
    free(cur);
}