#pragma once

#include <cstdint>
#include <vector>   // AJOUT

#include "../common/GLShader.h"

// A deplacer dans un entete specifique

struct vec3
{
    float x, y, z;
};

struct Vertex
{
    vec3 position;
    vec3 color;
};

// AJOUT : point de contrôle en pixels (comme la souris)
struct Point
{
    float x, y;
};

struct Application
{
    int32_t m_width;
    int32_t m_height;

    GLShader m_basicShader;

    // todo: creer une classe Mesh
    uint32_t m_VBO = 0;
    uint32_t m_IBO = 0;
    uint32_t m_VAO = 0;
    uint32_t m_indexCount = 0;
    uint32_t m_indexType = 0;
    uint32_t m_vertexCount = 0;

    // AJOUT : les points de contrôle P0, ..., Pn
    std::vector<Point> m_controlPoints;

    // AJOUT : nombre de pas pour tracer la courbe (pas = 1 / m_nbPas)
    int m_nbPas = 100;
    int m_nbPointsCourbe = 0;   // nombre de points de la courbe envoyés au GPU

    // AJOUT 2a : false = De Casteljau, true = Bernstein (triangle de Pascal)
    bool m_methodeBernstein = false;

    inline void setSize(int w, int h) { m_width = w; m_height = h; }

    bool initialize();
    void deinitialize();
    void update();
    void render();
};