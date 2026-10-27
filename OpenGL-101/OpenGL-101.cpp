// 1. definir les chemins vers les includes
// 2. definir les chemins vers les libraries
// 3. configurer en fonction de la plateforme (optionnel)

// glew doit toujours etre le premier include OpenGL
// comme on link en static, il faut le specifier
#define GLEW_STATIC 1
#include "GL/glew.h"
// ici w dans wglew est pour Windows
#include "GL/wglew.h"
#include <GLFW/glfw3.h>

#include "Application.h"
#include <iostream>   // AJOUT 2a : std::cout
#include <cmath>

// ----- AJOUT : clic gauche = ajouter un point de contrôle -----
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (app->m_modeSuppression)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            // Chercher le point le plus proche et le supprimer
            int closest = -1;
            float minDist = 15.f;
            for (int i = 0; i < (int)app->m_controlPoints.size(); i++)
            {
                float dx = app->m_controlPoints[i].x - (float)x;
                float dy = app->m_controlPoints[i].y - (float)y;
                float dist = sqrt(dx * dx + dy * dy);
                if (dist < minDist)
                {
                    minDist = dist;
                    closest = i;
                }
            }
            if (closest != -1)
            {
                app->m_controlPoints.erase(app->m_controlPoints.begin() + closest);
            }
        }
    }
    else
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            app->m_controlPoints.push_back(Point{ (float)x, (float)y });
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        {
            int closest = -1;
            float minDist = 15.f;
            for (int i = 0; i < (int)app->m_controlPoints.size(); i++)
            {
                float dx = app->m_controlPoints[i].x - (float)x;
                float dy = app->m_controlPoints[i].y - (float)y;
                float dist = sqrt(dx * dx + dy * dy);
                if (dist < minDist)
                {
                    minDist = dist;
                    closest = i;
                }
            }
            app->m_selectedPoint = closest;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        {
            app->m_selectedPoint = -1;
        }
    }
}
// ----- FIN AJOUT -----

// ----- AJOUT 2a : touche X = changer de méthode -----
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Application* app = (Application*)glfwGetWindowUserPointer(window);

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        app->m_methodeBernstein = !app->m_methodeBernstein;
        std::cout << "Methode : "
            << (app->m_methodeBernstein ? "Bernstein (Pascal)" : "De Casteljau")
            << std::endl;
    }

    if (key == GLFW_KEY_DELETE && action == GLFW_PRESS)
    {
        app->m_modeSuppression = true;
        std::cout << "Mode suppression : ON (clic gauche pour supprimer, ECHAP pour quitter)" << std::endl;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        app->m_modeSuppression = false;
        std::cout << "Mode suppression : OFF" << std::endl;
    }
    
    bool isPressed = (action == GLFW_PRESS);

    if (key == GLFW_KEY_UP)
        app->m_keyState.up = isPressed;
    if (key == GLFW_KEY_DOWN)
        app->m_keyState.down = isPressed;
    if (key == GLFW_KEY_LEFT)
        app->m_keyState.left = isPressed;
    if (key == GLFW_KEY_RIGHT)
        app->m_keyState.right = isPressed;

    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        app->m_modeTranslate = !app->m_modeTranslate;
        std::cout << "Mode TRANSLATE : " << (app->m_modeTranslate ? "ON" : "OFF") << std::endl;
    }

}
void cursorPosCallback(GLFWwindow* window, double x, double y)
{
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if (app->m_selectedPoint != -1)
    {
        app->m_controlPoints[app->m_selectedPoint].x = (float)x;
        app->m_controlPoints[app->m_selectedPoint].y = (float)y;
    }
}
// ----- FIN AJOUT 2a -----

int main(void)
{

    //d::cout << PascalTiangle(4,3) << '\n';


    Application app;

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 720, "OpenGL Bases", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // AJOUT : on accroche app à la fenêtre et on branche la souris
    glfwSetWindowUserPointer(window, &app);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);   // AJOUT 2a
    glfwSetCursorPosCallback(window, cursorPosCallback); // AJOUT

    app.initialize();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        app.setSize(width, height);

        app.update();
        /* Render here */
        app.render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    app.deinitialize();

    glfwTerminate();
    return 0;
}