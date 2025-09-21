#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <cmath>
#include <iostream>
#include <random>
#include <vector>

namespace
{
struct Particle
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    float life = 0.f;
    float maxLife = 1.f;
    float hueOffset = 0.f;
};

float length(const sf::Vector2f &v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(const sf::Vector2f &v)
{
    float len = length(v);
    if (len == 0.f)
    {
        return {0.f, 0.f};
    }
    return v / len;
}

float wrapAngle(float angle)
{
    constexpr float twoPi = 6.28318530718f;
    while (angle < 0.f)
    {
        angle += twoPi;
    }
    while (angle >= twoPi)
    {
        angle -= twoPi;
    }
    return angle;
}

sf::Color hsvToRgb(float h, float s, float v)
{
    h = wrapAngle(h) / (2.f * static_cast<float>(M_PI));
    float c = v * s;
    float x = c * (1.f - std::fabs(std::fmod(h * 6.f, 2.f) - 1.f));
    float m = v - c;

    sf::Color color;
    if (h < 1.f / 6.f)
        color = sf::Color(static_cast<sf::Uint8>((c + m) * 255), static_cast<sf::Uint8>((x + m) * 255), static_cast<sf::Uint8>(m * 255));
    else if (h < 2.f / 6.f)
        color = sf::Color(static_cast<sf::Uint8>((x + m) * 255), static_cast<sf::Uint8>((c + m) * 255), static_cast<sf::Uint8>(m * 255));
    else if (h < 3.f / 6.f)
        color = sf::Color(static_cast<sf::Uint8>(m * 255), static_cast<sf::Uint8>((c + m) * 255), static_cast<sf::Uint8>((x + m) * 255));
    else if (h < 4.f / 6.f)
        color = sf::Color(static_cast<sf::Uint8>(m * 255), static_cast<sf::Uint8>((x + m) * 255), static_cast<sf::Uint8>((c + m) * 255));
    else if (h < 5.f / 6.f)
        color = sf::Color(static_cast<sf::Uint8>((x + m) * 255), static_cast<sf::Uint8>(m * 255), static_cast<sf::Uint8>((c + m) * 255));
    else
        color = sf::Color(static_cast<sf::Uint8>((c + m) * 255), static_cast<sf::Uint8>(m * 255), static_cast<sf::Uint8>((x + m) * 255));

    return color;
}

float pseudoNoise2D(float x, float y)
{
    return std::sin(x * 1.7f + y * 2.3f) * 0.5f + std::cos(x * 0.5f - y * 1.9f) * 0.5f;
}

Particle makeParticle(const sf::Vector2f &center, std::mt19937 &rng, std::uniform_real_distribution<float> &dist)
{
    Particle p;
    float radius = dist(rng) * 200.f + 20.f;
    float angle = dist(rng) * 2.f * static_cast<float>(M_PI);
    p.position = center + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;
    p.velocity = sf::Vector2f(-std::sin(angle), std::cos(angle)) * (dist(rng) * 120.f + 50.f);
    p.life = 0.f;
    p.maxLife = dist(rng) * 6.f + 2.f;
    p.hueOffset = dist(rng) * 2.f * static_cast<float>(M_PI);
    return p;
}
}

int main()
{
    const unsigned int windowWidth = 1280;
    const unsigned int windowHeight = 720;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Neon Flux Garden", sf::Style::Close, settings);
    window.setVerticalSyncEnabled(true);

    if (!sf::Shader::isAvailable())
    {
        std::cerr << "Shader support not available on this device.\n";
        return 1;
    }

    sf::Shader backgroundShader;
    if (!backgroundShader.loadFromFile("resources/shaders/kaleido.frag", sf::Shader::Fragment))
    {
        std::cerr << "Failed to load fragment shader.\n";
        return 1;
    }

    std::cout << "\nNeon Flux Garden\n"
              << "==================\n"
              << "Controls:\n"
              << "  Mouse Move : Sculpt the energy field\n"
              << "  Left Click : Emit an energy burst\n"
              << "  Right Click: Toggle attraction / repulsion\n"
              << "  SPACE      : Toggle vortex turbulence\n"
              << "  C          : Toggle chroma cycling\n"
              << "  S          : Toggle slow-motion\n"
              << "  R          : Reset the particle universe\n"
              << "  ESC        : Quit\n";

    sf::Vector2u renderSize(windowWidth, windowHeight);
    sf::RenderTexture trailTexture;
    if (!trailTexture.create(renderSize.x, renderSize.y))
    {
        std::cerr << "Failed to create render texture.\n";
        return 1;
    }
    trailTexture.clear(sf::Color::Black);

    sf::RectangleShape fullscreenQuad(sf::Vector2f(static_cast<float>(renderSize.x), static_cast<float>(renderSize.y)));

    std::vector<Particle> particles;
    particles.reserve(2000);
    const std::size_t particleCount = 1200;

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist01(0.f, 1.f);

    sf::Vector2f center(renderSize.x / 2.f, renderSize.y / 2.f);
    for (std::size_t i = 0; i < particleCount; ++i)
    {
        particles.push_back(makeParticle(center, rng, dist01));
    }

    bool chromaCycling = true;
    bool turbulenceEnabled = true;
    bool attractMode = true;
    bool slowMotion = false;
    float time = 0.f;
    float chromaTime = 0.f;

    sf::Clock deltaClock;
    sf::Clock globalClock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::C)
                {
                    chromaCycling = !chromaCycling;
                }
                else if (event.key.code == sf::Keyboard::Space)
                {
                    turbulenceEnabled = !turbulenceEnabled;
                }
                else if (event.key.code == sf::Keyboard::R)
                {
                    particles.clear();
                    for (std::size_t i = 0; i < particleCount; ++i)
                    {
                        particles.push_back(makeParticle(center, rng, dist01));
                    }
                    trailTexture.clear(sf::Color::Black);
                }
                else if (event.key.code == sf::Keyboard::S)
                {
                    slowMotion = !slowMotion;
                }
            }
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Right)
                {
                    attractMode = !attractMode;
                }
            }
        }

        float dt = deltaClock.restart().asSeconds();
        if (slowMotion)
        {
            dt *= 0.25f;
        }
        time = globalClock.getElapsedTime().asSeconds();
        if (chromaCycling)
        {
            chromaTime += dt * 0.25f;
        }

        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        sf::Vector2f mouseFloat(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            for (int i = 0; i < 10; ++i)
            {
                particles.push_back(makeParticle(mouseFloat, rng, dist01));
            }
        }

        if (particles.size() > 5000)
        {
            particles.erase(particles.begin(), particles.begin() + (particles.size() - 5000));
        }

        sf::VertexArray particleVertices(sf::Points, particles.size());
        for (std::size_t i = 0; i < particles.size(); ++i)
        {
            Particle &p = particles[i];
            p.life += dt;
            if (p.life > p.maxLife)
            {
                p = makeParticle(mouseFloat, rng, dist01);
                p.life = 0.f;
            }

            sf::Vector2f direction = mouseFloat - p.position;
            float distance = length(direction) + 1.f;
            sf::Vector2f force = normalize(direction) * (120.f / distance);
            if (!attractMode)
            {
                force = -force;
            }

            if (turbulenceEnabled)
            {
                float n = pseudoNoise2D(p.position.x * 0.01f + time * 0.2f, p.position.y * 0.01f - time * 0.2f);
                sf::Vector2f perpendicular(-force.y, force.x);
                force += perpendicular * n * 0.5f;
            }

            p.velocity += force * dt * 60.f;
            p.velocity *= 0.96f;
            p.position += p.velocity * dt;

            if (p.position.x < -100.f || p.position.x > windowWidth + 100.f || p.position.y < -100.f || p.position.y > windowHeight + 100.f)
            {
                p = makeParticle(mouseFloat, rng, dist01);
            }

            float hue = p.hueOffset + chromaTime * 2.f;
            float saturation = 0.9f;
            float value = 0.9f;
            sf::Color color = hsvToRgb(hue, saturation, value);
            float alphaScale = std::max(0.f, 1.f - (p.life / p.maxLife));
            color.a = static_cast<sf::Uint8>(alphaScale * 255);

            particleVertices[i].position = p.position;
            particleVertices[i].color = color;
        }

        sf::RectangleShape fadeRect(sf::Vector2f(static_cast<float>(renderSize.x), static_cast<float>(renderSize.y)));
        fadeRect.setFillColor(sf::Color(0, 0, 0, slowMotion ? 10 : 35));

        trailTexture.draw(fadeRect, sf::BlendAlpha);
        trailTexture.draw(particleVertices, sf::BlendAdd);
        trailTexture.display();

        backgroundShader.setUniform("u_time", time * 0.2f);
        backgroundShader.setUniform("u_resolution", sf::Vector2f(static_cast<float>(renderSize.x), static_cast<float>(renderSize.y)));
        backgroundShader.setUniform("u_mouse", mouseFloat);
        backgroundShader.setUniform("u_energy", attractMode ? 1.f : -1.f);

        window.clear(sf::Color::Black);
        window.draw(fullscreenQuad, &backgroundShader);
        sf::Sprite trailSprite(trailTexture.getTexture());
        trailSprite.setColor(sf::Color(255, 255, 255, 220));
        window.draw(trailSprite, sf::BlendAdd);
        window.display();
    }

    return 0;
}
