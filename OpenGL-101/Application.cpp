#define GLEW_STATIC 1
#include "GL/glew.h"
// ici w dans wglew est pour Windows
#include "GL/wglew.h"
#include <iostream>
#include <array>
#include <cstddef>   // AJOUT : offsetof
#include "Application.h"
#include "Point.h"

// ----- AJOUT : algorithme de De Casteljau (version du cours) -----
// Entrée : les points de contrôle P0..Pn et une valeur de t entre 0 et 1
// Sortie : le point Q(t) de la courbe de Bézier
Point deCasteljau(const std::vector<Point> P, float t)
{
    if (t == 0) {
        return P[0];
    }
    int n = P.size() - 1;

    // Étape 0 : P_i^(0) = P_i  (on copie les points de contrôle)
    std::vector<Point> Q = P;

    // Pour j : 1 -> n
    for (int j = 1; j <= n; j++)
    {
        double x = Q[0].x;
        double y = Q[0].y;
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
/*  0   1   2   3 
0:  1
1:  1   1
2:  1   2   1
3:  1   3   3   1
4:  1   4   6   4   1
...
*/

int PascalTiangle(int i, int j) {
    if ((j == 0) || (j == i))
        return 1;
    else if (j > i)
        return 0;
    if (j == 1)
        return i;

    if (j > (i+1) / 2)
        return PascalTiangle(i, i - j);
    return PascalTiangle(i - 1, j - 1) + PascalTiangle(i - 1, j);
}

Point Bernstein(const std::vector<Point> P, float t) {
    Point Q = Point({
        0.f,0.f
        });
    int n = P.size() - 1;

    // Somme 0..i..n : Tp(n,i) t^i * (1-t)^(n-i)  * Pi
    for (int i = 0; i <= n; ++i) {
        const float _temp = PascalTiangle(n, i) * pow(t, i) * pow(1 - t, n - i);
        Q.x += _temp * P[i].x;
        Q.y += _temp * P[i].y;
    }

    return Q;
}

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
    const float cx = m_width / 2.f, cy = m_height / 2.f;
    const float sx = 1.5f, sy = 1.f;

    s_transform =
        Mat3::translation(cx, cy)
        * Mat3::rotate(1)
        * Mat3::scale(sx, sy)
        * Mat3::translation(0.f, 100.f)
        * Mat3::translation(-cx, -cy);

    auto toNDC = [&](const Point& p) {
        Point pp = s_transform * p;   // le pivot est déjà inclus dans s_transform
        return vec3{ 2.f * pp.x / m_width - 1.f, 1.f - 2.f * pp.y / m_height, 0.f };
    };

    // ----- AJOUT : on envoie les points de contrôle au GPU -----
    // Chaque point est mis 2 fois : en gris (polygone) puis en rouge (points)
    std::vector<Vertex> vertices;
    for (const Point& p : m_controlPoints)
    {
        // pixels -> coordonnées OpenGL entre -1 et 1 (y inversé)
        vec3 pp = toNDC(p);
        vertices.push_back(Vertex{ pp, vec3{0.4f, 0.4f, 0.4f} });
    }
    for (const Point& p : m_controlPoints)
    {
        vec3 pp = toNDC(p);
        vertices.push_back(Vertex{ pp, vec3{ 0.9f, 0.4f, 0.4f } });
    }

    // ----- points de la courbe de Bézier (De Casteljau), en Rouge -----
    time_t time = std::time(0);


    if (m_controlPoints.size() >= 2)
    {
        for (int k = 0; k <= m_nbPas; k++)
        {
            float t = (float)k / m_nbPas;
            Point q = deCasteljau(m_controlPoints, t);

            Point qp = s_transform * q;

            float x = 2.f * qp.x / m_width - 1.f;
            float y = 1.f - 2.f * qp.y / m_height;
            
                vertices.push_back(Vertex{ vec3{x + 0.01f, y, 0.f}, vec3{0.9f, 0.2f, 0.f} });
        }
    }
    //nst time_t casteljau = std::time(0) - time;

    //d::cout << "De casteljau : " << (std::time(0) - time)*100. << '\n';
    // ----- points de la courbe de Bézier (Bernstein), en Bleu -----
    time = std::time(0);

    if (m_controlPoints.size() >= 2)
    {
        for (int k = 0; k <= m_nbPas; k++)
        {
            float t = (float)k / m_nbPas;
            Point q = Bernstein(m_controlPoints, t);

            float x = 2.f * q.x / m_width - 1.f;
            float y = 1.f - 2.f * q.y / m_height;
            vertices.push_back(Vertex{ vec3{ x, y + 0.01f, 0.f }, vec3{ 0.1f, 0.2f, 0.9f } });
            m_nbPointsCourbe++;
        }
    }
    //nst time_t bernstein = std::time(0) - time;
    //td::cout << "nb : " << m_controlPoints.size() << "\ncastle : " << casteljau << "\nbernstein : " << bernstein << '\n';

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
