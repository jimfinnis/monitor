#include "app.h"
#include "exception.h"

Application *gApp;

int main(int argc, char *argv[])
{
    int rv=1;
    try {
        Application a(argc, argv);
        gApp = &a;
        rv = a.exec();
    } catch(Exception &e) {
        printf("Fatal error: %s\n",e.what());
    }
    return rv;
}
