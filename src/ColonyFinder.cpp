#include "ColonyFinder.h"

int main()
{
    AppController app;
    if (!app.Initialize())
    {
        return -1;
    }

    app.Run();
    return 0;
}
