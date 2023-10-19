#include "ellpack.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <numeric>

const int CELL_SIZE = 25;
using namespace sf;
template <typename T> void visualize(ELLpack<T> *ellpack) {
    RenderWindow window(VideoMode(ellpack->height() * CELL_SIZE, ellpack->height() * CELL_SIZE), "Mesh Visualizer");
    std::vector<ConvexShape> triangles;

    std::vector<Color> color_list;

    for (int i = 0; i < 255; i++) {
        int b = 255 - i;
        int r = i;
        color_list.push_back(Color(r, 0, b));
    }

    for (int i = 0; i < ellpack->size_total(); i += 2) {
        gen_triangle(ellpack, i, &triangles, &color_list);
    }

    bool start = false;
    while (window.isOpen()) {
        Event event;

        window.clear(Color::White);
        if (start) {
            ellpack->update();
            // ellpack->world.barrier();
            triangles.clear();

            for (int i = 0; i < ellpack->size_total(); i += 2) {
                gen_triangle(ellpack, i, &triangles, &color_list);
            }
        }
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::A) {
                    start = true;
                    // ellpack->update();
                    // ellpack->world.barrier();
                    // triangles.clear();

                    // for (int i = 0; i < ellpack->size_total(); i += 2) {
                    //     gen_triangle(ellpack, i, &triangles, &color_list);
                    // }
                }
            }
        }

        for (int i = 0; i < static_cast<int>(triangles.size()); i++) {
            window.draw(triangles[i]);
        }
        window.display();
    }
}

template <typename T>
void update_vis(RenderWindow &window, ELLpack<T> *ellpack, std::vector<ConvexShape> *triangles,
                std::vector<Color> *color_list) {
    if (window.isOpen()) {
        window.clear(Color::White);
        triangles->clear();
        for (int i = 0; i < ellpack->size_total(); i += 2) {
            gen_triangle(ellpack, i, triangles, color_list);
        }

        for (int i = 0; i < static_cast<int>(triangles->size()); i++) {
            window.draw((*triangles)[i]);
        }
        window.display();
    }
}
// genereate a triangle object
ConvexShape get_triangle(std::vector<Color> *color_list, int color_id, Vector2f p1, Vector2f p2, Vector2f p3) {
    ConvexShape triangle;
    Color color = (*color_list)[color_id];

    triangle.setPointCount(3);
    triangle.setPoint(0, p1);
    triangle.setPoint(1, p2);
    triangle.setPoint(2, p3);
    triangle.setFillColor(color);
    triangle.setOutlineColor(Color::Black);

    return triangle;
}

// add the two triangles for a cell
template <typename T>
void gen_triangle(ELLpack<T> *ellpack, int id, std::vector<ConvexShape> *triangles, std::vector<Color> *color_list) {
    int w = ellpack->width();
    float s = CELL_SIZE;
    Vector2f p1;
    Vector2f p2;
    Vector2f p3;
    Vector2f p4;

    if (id % w == 0) {
        p1 = {static_cast<float>(s * (id % w)), static_cast<float>(s * floor(id / w))};
    } else {
        p1 = (*triangles)[id - 1].getPoint(1);
    }
    p2 = {p1.x + s, p1.y};
    p3 = {p1.x, p1.y + s};
    p4 = {p2.x, p3.y};

    int c1 = normalize(ellpack, id, color_list->size());
    int c2 = normalize(ellpack, id + 1, color_list->size());

    triangles->push_back(get_triangle(color_list, c1, p1, p2, p3));
    triangles->push_back(get_triangle(color_list, c2, p4, p2, p3));
}

template <typename T> int normalize(ELLpack<T> *ellpack, int id, int s) {
    // double max = *std::max_element(ellpack->v_old.begin(), ellpack->v_old.end());
    double max = 1.0 / s;
    double min = 0; //*std::min_element(ellpack->v_new.begin(), ellpack->v_new.end());
    double range = max - min;

    int color_id = static_cast<int>(floor(((ellpack->v_old[id] - min) / range) * s));

    // Ensure color_index is within the valid range of color_list
    // if (color_id < 0) {
    //     color_id = 0;
    // }
    if (color_id >= s) {
        color_id = s - 1;
    }

    return color_id;
}
