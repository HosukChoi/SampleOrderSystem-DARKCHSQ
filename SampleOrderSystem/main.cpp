#include <gtest/gtest.h>
#include <gmock/gmock.h>

#ifdef RUN_APP
#include "controller/AppController.h"
#endif

int main(int argc, char* argv[]) {
#ifdef RUN_APP
    AppController app;
    app.run();
    return 0;
#else
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
#endif
}
