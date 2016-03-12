#include "HathClient.h"

int main(int argc, char *argv[])
{
    HathClient &app = HathClient::instanse();
    return app.run(argc, argv);
}