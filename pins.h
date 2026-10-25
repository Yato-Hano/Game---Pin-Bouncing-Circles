#pragma once
#include <SFML/Graphics.hpp>
import std;
/*
   Pin moving bouncing circles with mouse clicks but beware of bombs, the timer, and finite pins. 
*/
inline int rand_int(int min, int max)
{
    static std::default_random_engine ran{ unsigned(time(0)) };
    return std::uniform_int_distribution<>{min, max}(ran);
}

sf::Font font{ "OpenSans.ttf" };

class Bouncing_Circle : public sf::Drawable
{
public:
    Bouncing_Circle();
    const sf::Vector2f& get_velocity() const { return velocity; }
    bool is_moving() const { return velocity.x != 0 || velocity.y != 0; }

    virtual bool is_this_circle() const { return true; }
    virtual float get_radius() const { return circle.getRadius(); }
    virtual sf::Vector2f get_position() const { return circle.getPosition(); }
    virtual void stop() { velocity = {}; }
    virtual void restore_position() { circle.setPosition(saved_position); };
    virtual void set_position(const sf::Vector2f& p) { circle.setPosition(p); }
    virtual void move_by(const sf::Vector2f& offset) { circle.move(offset); }

    void x_inversion() { velocity.x = -velocity.x; }
    void y_inversion() { velocity.y = -velocity.y; }
    void add_velocity(sf::Vector2f dv) { velocity += dv; }

    friend void move(Bouncing_Circle&, const sf::Vector2u& bounds);
    friend void bounce(Bouncing_Circle&, Bouncing_Circle&);
protected:
    sf::Vector2f saved_position;
    sf::Vector2f velocity;
private:
    sf::CircleShape circle;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(circle, states);
    }
};


// origin and radius hardcoded for the exact PNG
class Bouncing_Bomb : public Bouncing_Circle
{
public:
    Bouncing_Bomb(const sf::Texture& t)
        : sprite{ t }
    {
        sprite.setOrigin({ 50,66 });
    }

    bool is_this_circle() const override { return false; }
    float get_radius() const override { return 33; }
    sf::Vector2f get_position() const override { return sprite.getPosition(); }
    void stop() override { Bouncing_Circle::stop(); sprite.setColor(sf::Color::Red); }
    void restore_position() override { sprite.setPosition(saved_position); };
    void set_position(const sf::Vector2f& p) override { sprite.setPosition(p); }
    void move_by(const sf::Vector2f& xy) override { sprite.move(xy); }

private:
    sf::Sprite sprite;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(sprite, states);
    }
};

class Timer : public sf::Drawable
{
    sf::Clock clock;
    float max_seconds;
    sf::Text text;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(text, states);
    }
public:
    Timer(float s);
    void restart() { clock.restart(); }
    void set_text_formatted(const std::string& message);
    void stop() { clock.stop(); }
    bool does_ticks() { return clock.isRunning(); }
    void operator()()
    {
        float seconds_left{ max_seconds - clock.getElapsedTime().asSeconds() };
        if (seconds_left <= 0) {
            stop(); set_text_formatted("You Loose.");
        }
        else set_text_formatted(""); // continue playing
    }
};
//--------------------------------------------------------------------------------------------

class Pins_Pocket : public sf::Drawable
{
public:
    Pins_Pocket(unsigned pins_max, Timer& t);
    bool empty() const { return pins_left ? false : true; }
    void remove_one();
    void hit_remove_one(sf::Vector2f hit_position);
    void setPosition(sf::Vector2f p) { pin.setPosition(p); }
private:
    unsigned pins_left;
    sf::Text text;
    sf::Texture pin_texture;
    sf::Sprite pin;
    std::vector<sf::Sprite> pins_pinned;
    Timer& timer;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(text, states);
        for (const auto& p : pins_pinned)
            target.draw(p, states);
        target.draw(pin, states);
    }
};

class Move_Handler
{
    sf::RenderWindow& window;
    std::vector<std::unique_ptr<Bouncing_Circle>>& shapes;
    Pins_Pocket& pins;
    Timer& timer;
public:
    Move_Handler(sf::RenderWindow& w, std::vector<std::unique_ptr<Bouncing_Circle>>& v, Pins_Pocket& p, Timer& timer);
    void operator()(const sf::Event::MouseMoved& m);
    bool is_any_mooving(); // except bombs
    void operator()(const sf::Event::MouseButtonPressed& m); 
    void operator()(); //move andbounce. call every frame
};

class Window :public sf::RenderWindow
{
public:
    Window();
    void show();
private:
    Timer timer;
    size_t max_circles{ 13 };
    std::vector<std::unique_ptr<Bouncing_Circle>> circles;
    Pins_Pocket pins;
    Move_Handler handler;
    sf::Texture bomb_texture;
};