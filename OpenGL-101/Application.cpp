#define GLEW_STATIC 1
#include "GL/glew.h"
// ici w dans wglew est pour Windows
#include "GL/wglew.h"

#include <array>
#include <cstddef>   // AJOUT : offsetof
#include "Application.h"

// ----- AJOUT : algorithme de De Casteljau (version du cours) -----
// Entrée : les points de contrôle P0..Pn et une valeur de t entre 0 et 1
// Sortie : le point Q(t) de la courbe de Bézier
Point deCasteljau(const std::vector<Point>& P, float t)
{
    int n = (int)P.size() - 1;

    // Étape 0 : P_i^(0) = P_i  (on copie les points de contrôle)
    std::vector<Point> Q = P;

    // Pour j : 1 -> n
    for (int j = 1; j <= n; j++)
    {
        // Pour i : 0 -> n-j
        for (int i = 0; i <= n - j; i++)
        {
            // P_i^(j) = (1-t) P_i^(j-1) + t P_{i+1}^(j-1)
            Q[i].x = (1 - t) * Q[i].x + t * Q[i + 1].x;
            Q[i].y = (1 - t) * Q[i].y + t * Q[i + 1].y;
        }
    }

    // Sortie : Q(t) = P_0^(n)
    return Q[0];
}
// ----- FIN AJOUT -----

bool Application::initialize()
{
    GLenum error = glewInit();
    if (error != GLEW_OK)
        return false;

    m_basicShader.LoadVertexShader("basic.vs.glsl");
    m_basicShader.LoadFragmentShader("basic.fs.glsl");
    m_basicShader.Create();

    const std::array<Vertex, 4> quadVertices{
        Vertex{vec3{-0.8f, -0.8f, 0.f}, vec3{1.f, 0.f, 0.f}},
        Vertex{vec3{+0.8f, -0.8f, 0.f}, vec3{0.f, 1.f, 0.f}},
        Vertex{vec3{+0.8f, +0.8f, 0.f}, vec3{0.f, 0.f, 1.f}},
        Vertex{vec3{-0.8f, +0.8f, 0.f}, vec3{1.f, 1.f, 1.f}}
    };
    m_vertexCount = quadVertices.size();

    const std::array<uint16_t, 6> quadIndices{ 0, 1, 2, 0, 2, 3 };
    m_indexCount = quadIndices.size();
    m_indexType = GL_UNSIGNED_SHORT;

    // ----- AJOUT : VAO + VBO pour les points de contrôle -----
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // on demande au shader où sont ses attributs
    uint32_t program = m_basicShader.GetProgram();
    GLint locPosition = glGetAttribLocation(program, "a_Position");
    GLint locColor = glGetAttribLocation(program, "a_Color");

    glEnableVertexAttribArray(locPosition);
    glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (const void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(locColor);
    glVertexAttribPointer(locColor, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (const void*)offsetof(Vertex, color));

    glBindVertexArray(0);
    glPointSize(8.f);
    // ----- FIN AJOUT -----

    return true;
}

void Application::deinitialize()
{
    // AJOUT
    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);

    m_basicShader.Destroy();

}

void Application::update()
{
    // ----- AJOUT : on envoie les points de contrôle au GPU -----
    // Chaque point est mis 2 fois : en gris (polygone) puis en rouge (points)
    std::vector<Vertex> vertices;
    for (const Point& p : m_controlPoints)
    {
        // pixels -> coordonnées OpenGL entre -1 et 1 (y inversé)
        float x = 2.f * p.x / m_width - 1.f;
        float y = 1.f - 2.f * p.y / m_height;
        vertices.push_back(Vertex{ vec3{x, y, 0.f}, vec3{0.4f, 0.4f, 0.4f} });
    }
    for (const Point& p : m_controlPoints)
    {
        float x = 2.f * p.x / m_width - 1.f;
        float y = 1.f - 2.f * p.y / m_height;
        vertices.push_back(Vertex{ vec3{x, y, 0.f}, vec3{0.9f, 0.1f, 0.1f} });
    }

    // ----- AJOUT : points de la courbe de Bézier (De Casteljau), en bleu -----
    // Pour t : 0 -> 1, pas = 1 / m_nbPas
    m_nbPointsCourbe = 0;
    if (m_controlPoints.size() >= 2)
    {
        for (int k = 0; k <= m_nbPas; k++)
        {
            float t = (float)k / m_nbPas;
            Point q = deCasteljau(m_controlPoints, t);

            float x = 2.f * q.x / m_width - 1.f;
            float y = 1.f - 2.f * q.y / m_height;
            vertices.push_back(Vertex{ vec3{x, y, 0.f}, vec3{0.1f, 0.2f, 0.9f} });
            m_nbPointsCourbe++;
        }
    }
    // ----- FIN AJOUT -----

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        vertices.data(), GL_DYNAMIC_DRAW);
    // ----- FIN AJOUT -----
}

void Application::render()
{
    glViewport(0, 0, m_width, m_height);
    glClearColor(1.f, 1.f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    uint32_t program = m_basicShader.GetProgram();
    glUseProgram(program);

    // ----- AJOUT : dessin du polygone de contrôle -----
    GLsizei n = (GLsizei)m_controlPoints.size();
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_LINE_STRIP, 0, n);   // le polygone (sommets gris)
    glDrawArrays(GL_LINE_STRIP, 2 * n, m_nbPointsCourbe);   // AJOUT : la courbe (sommets bleus), points reliés 2 à 2
    glDrawArrays(GL_POINTS, n, n);       // les points  (sommets rouges)
    glBindVertexArray(0);
    // ----- FIN AJOUT -----
}