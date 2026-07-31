#include "application/application.hpp"

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "MAIN::ERROR:: log path absent!" << std::endl;
        return 1;
    }
    
    Application app(5555, 30, 5000, 50000);

    app.run(argv[0]);
    
    return 0;
}