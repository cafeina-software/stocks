#ifndef APP_H
#define APP_H

#include <Application.h>

class App : public BApplication {
public:
  App();

private:
  BWindow *window{};
};

#endif
