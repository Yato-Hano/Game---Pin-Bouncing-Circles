#pragma once

/*
   Pin moving bouncing circles with mouse clicks, 
   but beware of bombs, the timer, and finite pins.
*/

#include <SFML/Graphics.hpp>
#include <format>
#include <print>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

inline int rand_int(int min, int max)
{
    static std::default_random_engine ran{ unsigned(time(0)) };
    return std::uniform_int_distribution<>{min, max}(ran);
}
sf::Font font{ "OpenSans.ttf" };
//--------------------------------------------------------------------------------------------
enum class  Object_Type { Circle, Bomb };

class Bouncing_Object : public sf::Drawable
{
public:
    Bouncing_Object() = default;
    const sf::Vector2f& get_velocity() const { return velocity; }
    bool is_moving() const { return velocity.x != 0 || velocity.y != 0; }

    virtual Object_Type object_type() const = 0;
    virtual float get_radius() const = 0;
    virtual sf::Vector2f get_position() const = 0;
    virtual void stop() { velocity = {}; }
    virtual void restore_position() = 0;
    virtual void set_position(const sf::Vector2f& p) = 0;

    void save_position() { saved_position = get_position(); }
    void x_inversion() { velocity.x = -velocity.x; }
    void y_inversion() { velocity.y = -velocity.y; }
    void add_velocity(sf::Vector2f dv) { velocity += dv; }

    friend void move(Bouncing_Object&, const sf::Vector2u& bounds); // use this to move an object
    friend void bounce(Bouncing_Object&, Bouncing_Object&);
protected:
    virtual void move_by(const sf::Vector2f& offset) = 0;
    sf::Vector2f saved_position;
private:
    sf::Vector2f velocity{ 1.5f, 1.0f };
};
//--------------------------------------------------------------------------------------------
class Bouncing_Circle : public Bouncing_Object
{
public:
    Bouncing_Circle();

    Object_Type object_type() const override {   return Object_Type::Circle;   }
    float get_radius() const override { return circle.getRadius(); }
    sf::Vector2f get_position() const override { return circle.getPosition(); }

    void restore_position() override { circle.setPosition(saved_position); };
    void set_position(const sf::Vector2f& p) override { circle.setPosition(p); }
protected:
    void move_by(const sf::Vector2f& offset) override {circle.move(offset); }
private:
    sf::CircleShape circle;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(circle, states);
    }
};
//--------------------------------------------------------------------------------------------
// origin and radius hardcoded for the exact PNG
class Bouncing_Bomb : public Bouncing_Object
{
public:
    Bouncing_Bomb(const sf::Texture& t);

    Object_Type object_type() const override { return Object_Type::Bomb; }
    float get_radius() const override { return 33; }
    sf::Vector2f get_position() const override { return sprite.getPosition(); }

    void stop() override { Bouncing_Object::stop(); sprite.setColor(sf::Color::Red); }
    void restore_position() override { sprite.setPosition(saved_position); };
    void set_position(const sf::Vector2f& p) override { sprite.setPosition(p); }
protected:
    void move_by(const sf::Vector2f& xy) override {  sprite.move(xy); }
private:
    sf::Sprite sprite;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(sprite, states);
    }
};
//--------------------------------------------------------------------------------------------
class Timer : public sf::Drawable
{
public:
    Timer(float s, sf::Text& gs);
    void restart() { clock.restart(); }
    void set_text_formatted(const std::string& message);
    void stop() { clock.stop(); }
    bool does_ticks() { return clock.isRunning(); }
    void operator()(); // tick the timer
private:
    sf::Clock clock;
    float max_seconds;
    sf::Text& game_status;
    sf::Text text;
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(text, states);
    }
};
//--------------------------------------------------------------------------------------------
class Pins_Pocket : public sf::Drawable
{
public:
    Pins_Pocket(unsigned pins_max, sf::Text& t, Timer&);
    bool empty() const { return pins_left ? false : true; }
    void remove_one(); // prints number of pins left as well
    void hit_remove_one(sf::Vector2f hit_position); 
    void setPosition(sf::Vector2f p) { pin.setPosition(p); }
private:
    unsigned pins_left;
    sf::Text& game_status;
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
//--------------------------------------------------------------------------------------------
class Move_Handler
{
public:
    Move_Handler(sf::Vector2u b, sf::Text& gs, std::vector<std::unique_ptr<Bouncing_Object>>& v, Pins_Pocket& p, Timer& timer);
    void operator()(const sf::Event::MouseMoved& m);
    bool is_any_mooving(); // except bombs
    void operator()(const sf::Event::MouseButtonPressed& m); 
    void operator()(); //move and bounce. call every frame
private:
    sf::Vector2u bounds;
    sf::Text& game_status;
    std::vector<std::unique_ptr<Bouncing_Object>>& shapes;
    Pins_Pocket& pins;
    Timer& timer;
};
//--------------------------------------------------------------------------------------------
class Window :public sf::RenderWindow
{
public:
    Window();
    void show();
private:
    Timer timer;
    unsigned max_circles{ 13 };
    std::vector<std::unique_ptr<Bouncing_Object>> circles;
    Pins_Pocket pins;
    Move_Handler handler;
    sf::Texture bomb_texture;
    sf::Text game_status;
};
//--------------------------------------------------------------------------------------------
