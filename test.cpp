#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(2000, 1000), "SFML window");

    // Create a graphical text to display


    int x=0;
    // Start the game loop
    window.display();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    while (x<255)
    {   x++;
        sf::CircleShape s2(100+x);
        s2.setFillColor(sf::Color{255, 0, 0, x});
        s2.setPosition(50+x, -50+x);
        sf::CircleShape s1(400-x);
        s1.setFillColor(sf::Color{0, 255, 255, 255-x});
        s1.setPosition(450-2*x, 350-2*x);
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear screen
        window.clear();

        // Draw the shape
        window.draw(s1);
        window.draw(s2);

        // Update the window
        window.display();

        window.draw(s2);

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    return 0;
}
