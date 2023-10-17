#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client_stub.h"
#include "data.h"
#include "entry.h"

#define MAX_COMMAND_LENGTH 1024

void print_result(struct data_t *result) {
    if (result != NULL) {
        printf("Result: %s\n", data_to_string(result));
    } else {
        printf("Result: NULL\n");
    }
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname:port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct rtable_t *rtable = rtable_connect(argv[1]);
    if (rtable == NULL) {
        fprintf(stderr, "Failed to connect to the server.\n");
        exit(EXIT_FAILURE);
    }

    char command[MAX_COMMAND_LENGTH];
    char *token;

    while (1) {
        printf("Enter a command: ");
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            break;
        }

        // Remove newline character from the input
        command[strcspn(command, "\n")] = 0;

        token = strtok(command, " ");

        if (token == NULL) {
            continue;  // Empty command, ask for a new one
        }

        if (strcmp(token, "put") == 0) {
            // Parse the put command
            char *key = strtok(NULL, " ");
            char *data = strtok(NULL, "");
            if (key != NULL && data != NULL) {
                struct data_t *value = data_create2(strlen(data), data);
                struct entry_t *entry = entry_create(key, value);
                int result = rtable_put(rtable, entry);
                if (result == 0) {
                    printf("Put operation successful.\n");
                } else {
                    printf("Put operation failed.\n");
                }
            }

        } else if (strcmp(token, "get") == 0) {
            // Parse the get command
            char *key = strtok(NULL, " ");
            if (key != NULL) {
                struct data_t *result = rtable_get(rtable, key);
                print_result(result);
            }

        } else if (strcmp(token, "del") == 0) {
            // Parse the del command
            char *key = strtok(NULL, " ");
            if (key != NULL) {
                int result = rtable_del(rtable, key);
                if (result == 0) {
                    printf("Delete operation successful.\n");
                } else {
                    printf("Delete operation failed.\n");
                }
            }

        } else if (strcmp(token, "size") == 0) {

            int size = rtable_size(rtable);
            printf("Table size: %d\n", size);

        } else if (strcmp(token, "getkeys") == 0) {

            char **keys = rtable_get_keys(rtable);
            if (keys != NULL) {
                printf("Keys:\n");
                for (int i = 0; keys[i] != NULL; i++) {
                    printf("%s\n", keys[i]);
                }
                rtable_free_keys(keys);
            } else {
                printf("Failed to get keys.\n");
            }

        } else if (strcmp(token, "gettable") == 0) {

            struct entry_t **entries = rtable_get_table(rtable);
            if (entries != NULL) {
                printf("Table Entries:\n");
                for (int i = 0; entries[i] != NULL; i++) {
                    printf("Key: %s, Value: %s\n", entries[i]->key, data_to_string(entries[i]->value));
                }
                rtable_free_entries(entries);
            } else {
                printf("Failed to get table entries.\n");
            }

        } else if (strcmp(token, "quit") == 0) {

            rtable_disconnect(rtable);
            break;

        } else {
            printf("Invalid command. Available commands: put, get, del, size, getkeys, gettable, quit\n");
        }
    }

    return 0;
}
