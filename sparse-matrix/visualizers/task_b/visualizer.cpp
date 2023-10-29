#include "ellpack.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <numeric>

// Function prototypes
const int CELL_SIZE = 25;
using namespace sf;
template <typename T> int normalize(T value, int s);
template <typename T>
void generate(std::vector<T> *v_old, int w, VertexArray *triangles, std::vector<Color> *color_list);
template <typename T> void update_tri(std::vector<T> *v_old, std::vector<Color> *color_list, Vertex *v1, int id);

template <typename T> void visualize(ELLpack<T> *ellpack) {
    RenderWindow window(VideoMode(ellpack->width() / 2 * CELL_SIZE, ellpack->height() * CELL_SIZE), "Mesh Visualizer");
    VertexArray triangles(Triangles);
    Clock dt_clock, fps;
    float lastTime = 0;

    std::vector<Color> color_list;

    for (int i = 0; i < 255; i++) {
        int b = 255 - i;
        int r = i;
        color_list.push_back(Color(r, 0, b));
    }

    generate(&(ellpack->v_old), ellpack->width(), &triangles, &color_list);

    // bool start = false;
    while (window.isOpen()) {
        Event event;

        // if (start) {
        // ellpack->update();

        // for (int i = 0; i < (int)triangles.getVertexCount(); i++) {
        //     update_tri(&(ellpack->v_old), &color_list, &triangles[i], i);
        // }
        // }

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::A) {
                    ellpack->update();

                    for (int i = 0; i < (int)triangles.getVertexCount(); i++) {
                        update_tri(&(ellpack->v_old), &color_list, &triangles[i], i);
                    }
                    // start = true;
                }
            }
        }

        window.draw(triangles);
        window.display();
        float dt = dt_clock.restart().asSeconds();
        if (fps.getElapsedTime().asSeconds() > 1) {
            fps.restart();
            std::cout << ((1.0 / dt > 60) ? 60 : (1.0 / dt)) << std::endl;
        }
    }
}

template <typename T> void update_tri(std::vector<T> *v_old, std::vector<Color> *color_list, Vertex *v1, int id) {
    int color = normalize((*v_old)[floor(id / 3)], color_list->size());
    v1->color = (*color_list)[color];
}

template <typename T>
void generate(std::vector<T> *v_old, int w, VertexArray *triangles, std::vector<Color> *color_list) {
    float s = CELL_SIZE;

    for (int i = 0; i < static_cast<int>(v_old->size()); i += 2) {
        Vector2f p1;
        Vector2f p2;
        Vector2f p3;
        Vector2f p4;
        if (i % w == 0) {
            p1 = {0, static_cast<float>(s * floor(i / w))};
        } else {
            p1 = (*triangles)[(i * 3) - 1].position;
        }

        p2 = {p1.x + s, p1.y};
        p3 = {p1.x, p1.y + s};
        p4 = {p2.x, p3.y};

        int c1 = normalize((*v_old)[i], color_list->size());
        int c2 = normalize((*v_old)[i + 1], color_list->size());

        triangles->append(Vertex(p1, (*color_list)[c1]));
        triangles->append(Vertex(p2, (*color_list)[c1]));
        triangles->append(Vertex(p3, (*color_list)[c1]));
        triangles->append(Vertex(p3, (*color_list)[c2]));
        triangles->append(Vertex(p4, (*color_list)[c2]));
        triangles->append(Vertex(p2, (*color_list)[c2]));
    }
}

template <typename T> int normalize(T value, int s) {
    double range = 1.0 / s;
    int color_id = static_cast<int>(floor((value / range) * s));

    if (color_id < 0) {
        color_id = 0;
    } else if (color_id >= s) {
        color_id = s - 1;
    }

    return color_id;
}
