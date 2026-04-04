#include "app.hpp"

int main()
{
    viexpr::app app("ViExpr – Expression Visualizer", 1000, 600);

    app.setup();
    app.run();

    return 0;
}
