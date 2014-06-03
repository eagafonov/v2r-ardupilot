#include <GPSDClient.h>
#include <logger.h>

int main(int, char**) {

    GPSDClient client;
    client.thread_proc();

    return 0;
}