#include "define.h"

int listen_port = DEFAULT_LISTEN_PORT;
int address_length = sizeof (struct sockaddr_in);

char database_path[DATABASE_PATH_LENGTH] = DEFAULT_DATABASE_PATH;

CarTypeNode *head = NULL;

CarTypeNode *ct_ptr = NULL;
CarInfoNode *ci_ptr = NULL;
RentOrderNode *ro_ptr = NULL;

char json_buffer[JSON_BUFFER_LENGTH];

char command_buffer[COMMAND_BUFFER_LENGTH];

int is_saved = 1;