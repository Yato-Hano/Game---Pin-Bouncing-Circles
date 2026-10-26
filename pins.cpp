#include <pins.h>

//--------------------------------------------
Bouncing_Bomb::Bouncing_Bomb(const sf::Texture& t)
    : sprite{ t }
{
    sprite.setOrigin({ 50,66 });
    save_position();
}

Bouncing_Circle::Bouncing_Circle()
{
    auto radius{ rand_int(10, 70) };
    circle.setRadius(radius);
    circle.setOrigin(sf::Vector2f(radius, radius));
    circle.setFillColor(sf::Color(rand_int(10, 255), rand_int(10, 255), rand_int(10, 255)));
    circle.setOutlineThickness(-3);
    circle.setOutlineColor(sf::Color::Cyan);

    save_position();
}

void bounce(Bouncing_Object& a, Bouncing_Object& b)
{
    if (&a == &b) return;

    const sf::Vector2f delta{ a.get_position() - b.get_position() };
    const float dist{ delta.length() };
    const float radius_sum{ a.get_radius() + b.get_radius() };

    if (dist >= radius_sum)
        return;                                         // also replaces circles_collide

    const sf::Vector2f normal{ delta / dist };            // from b to a

    // exchange velocity along the normal, only if they are approaching
    const float approach{ (a.get_velocity() - b.get_velocity()).dot(normal) };
    if (approach < 0)
    {
        if (a.is_moving())
        {
            a.add_velocity(-normal * approach);
            a.restore_position();
        }
        if (b.is_moving())
        {
            b.add_velocity(normal * approach);
            b.restore_position();
        }
    }
}
void move(Bouncing_Object& object, const sf::Vector2u& bounds)
{
    object.save_position();
    object.move_by(object.velocity);

    float radius{ object.get_radius() };
    sf::Vector2f pos{ object.get_position() };

    if ((pos.x - radius) < 0 || bounds.x < (pos.x + radius)) {
        object.x_inversion();
        object.restore_position();
    }
    if ((pos.y - radius < 0) || (bounds.y < pos.y + radius)) {
        object.y_inversion();
        object.restore_position();
    }
}
//--------------------------------------------------------------------------------------------

Pins_Pocket::Pins_Pocket(unsigned pins_max, sf::Text& gs,Timer& t) :
    pins_left{ pins_max },
    game_status{gs},
    text{ font,std::format("{} pins", pins_left) },
    pin_texture{ "pin.PNG" },
    pin{ pin_texture },
    timer{t}
{
    text.setPosition({ 700, 0 });
    text.setFillColor(sf::Color::Red);
    pin.setScale({ 0.05f, 0.05f });
    pin.setOrigin(pin.getLocalBounds().getCenter());
    pins_pinned.reserve(pins_max);
}
void Pins_Pocket::remove_one()
{
    --pins_left;
    if (empty())
    {
        timer.stop();
        game_status.setString("You Loose");
        text.setString("Empty.");
        return;
    }
    text.setString(std::format("{} pins", pins_left));
}
void Pins_Pocket::hit_remove_one(sf::Vector2f hit_position)
{
    remove_one();
    pins_pinned.emplace_back(pin);
    pins_pinned.back().setPosition(hit_position);
    pins_pinned.back().scale({ 0.5,0.5 });
}
//--------------------------------------------

bool circles_collide(const Bouncing_Object& a, const Bouncing_Object& b)
{
    if (&a == &b) return false;
    return (a.get_position() - b.get_position()).length() <= a.get_radius() + b.get_radius();
}

//--------------------------------------------
Timer::Timer(float s, sf::Text& gs) :
    max_seconds{ s },
    game_status(gs),
    text{ font,"timer-here" }
{
    text.setPosition({ 0, 0 });
    text.setFillColor(sf::Color::Yellow);
}
void Timer::set_text_formatted(const std::string& message)
{
    float seconds_left{ max_seconds - clock.getElapsedTime().asSeconds() };
    text.setString(std::format("{:.3}s {}", seconds_left < 0 ? 0 : seconds_left, message));
}
void Timer::operator()()
{
    float seconds_left{ max_seconds - clock.getElapsedTime().asSeconds() };

    if (seconds_left <= 0) 
    {
        stop();
        text.setString("Time Is Over.");
        game_status.setString("You Loose.");
    }
    else
    set_text_formatted(""); // print time
}
//--------------------------------------------
Move_Handler::Move_Handler(sf::Vector2u b, sf::Text& gs, std::vector<std::unique_ptr<Bouncing_Object>>& v, Pins_Pocket& p, Timer& timer)
    :bounds{ b }, game_status{ gs }, shapes{ v }, pins{ p }, timer{ timer }
{
}
bool Move_Handler::is_any_mooving() {
    for (const auto& c : shapes)
    {
        if (c->object_type() == Object_Type::Circle && c->is_moving()) // allow moving bombs
        {
            return true;
        }
    }
    return false;
}
void Move_Handler::operator()(const sf::Event::MouseButtonPressed& m)
{
    if (!timer.does_ticks()) return;

    for (auto& c : shapes)
    {
        sf::Vector2f pos = sf::Vector2f(m.position);
        if ((pos - c->get_position()).length() <= c->get_radius()) // is circle clicked
        {
            if (c->is_moving())
            {
                c->stop(); // pined
                if (c->object_type() == Object_Type::Bomb)
                {
                    timer.stop();
                    game_status.setString("You Loose. Boom.");
                }
                else
                    if (!is_any_mooving()) // is every circle clicked
                    {
                        timer.stop();
                        game_status.setString("Win");
                    }
            }

            pins.hit_remove_one(pos); // pin the circle
            return;
        }
    }
    pins.remove_one();
}
void Move_Handler::operator()(const sf::Event::MouseMoved& m)
{
    pins.setPosition(static_cast<sf::Vector2f>(m.position));
}
void Move_Handler::operator()()
{
    for (auto& c : shapes)
        move(*c, bounds);

    for (size_t i{ 0 }; i < shapes.size(); i++)
    {
        for (size_t n{ i + 1 }; n < shapes.size(); n++)
        {
            bounce(*shapes[i], *shapes[n]);
        }
    }
}
//--------------------------------------------
Window::Window()
    :sf::RenderWindow{ sf::VideoMode{ { 800, 800 } }, "SFML Hello",sf::Style::Close ,sf::State::Windowed,sf::ContextSettings{.antiAliasingLevel = 8 } }, 
    timer{10.f,game_status },
    pins{max_circles+5,game_status,timer},
    handler{ getSize(),game_status, circles,pins,timer},
    bomb_texture{"bomb100.png"},
    game_status{font,"Not Over"}
{
    setFramerateLimit(60);

    game_status.setFillColor(sf::Color::White);
    game_status.setPosition({300,0});

    circles.reserve(max_circles);
    for (size_t i = 0; i < max_circles; i++)
    {
        if (i % 6 == 0)
            circles.push_back(std::make_unique<Bouncing_Bomb>(bomb_texture));
        else
            circles.push_back(std::make_unique<Bouncing_Circle>());
       
        circles.back()->set_position(sf::Vector2f(rand_int(100, 700), rand_int(100, 700)));

        // a little bit to randomise movement speed and direction
        float sign_x = (i % 2)? -1.f : 1.f;
        float sign_y = (i % 2) ? -1.f : 1.f;
        float x = rand_int(1, 7)/10.f;
        float y = rand_int(1, 7)/10.f;
        circles.back()->add_velocity({ sign_x * x,sign_y * y});
        if (sign_x < 0)  circles.back()->x_inversion();
        if (sign_y < 0)  circles.back()->y_inversion();
    }

    for (size_t i{ 0 }; i < circles.size(); i++)
    {
        for (size_t n{ 0 }; n < circles.size(); n++)
        {
            if (circles_collide(*circles[i], *circles[n]))
            {
                circles[i]->set_position(sf::Vector2f(rand_int(100, 700), rand_int(100, 700)));
                n = 0;
            }
        }
    }
}
void Window::show()
{
    timer.restart();

    while (isOpen())
    {
        handleEvents([this](const sf::Event::Closed) { close(); }, handler);
        
        timer(); 
        handler(); 
      
        clear(sf::Color{ 18, 20, 32 });
        for (const auto& c : circles) 
        {
            draw(*c);
        }
        draw(timer);
        draw(game_status);
        draw(pins);

        display();
    }
}
//--------------------------------------------

int main()
try
{
   Window w;
   w.show();
}
catch(const std::exception& e)
{
    std::ofstream ofs{"error_log.txt",std::ios::app};
    ofs << "Error: " << e.what() <<'.' << std::endl;
    std::println("Error: {}.", e.what());
    std::cin.get();
}
catch(...)
{
    std::ofstream ofs{ "error_log.txt",std::ios::app };
    ofs << "Some error." << std::endl;
    std::println("Some error.");
    std::cin.get();
}

