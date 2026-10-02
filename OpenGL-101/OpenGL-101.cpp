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

// ----- AJOUT : clic gauche = ajouter un point de contrôle -----
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        Application* app = (Application*)glfwGetWindowUserPointer(window);
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        app->m_controlPoints.push_back(Point{ (float)x, (float)y });
    }
}
// ----- FIN AJOUT -----

// ----- AJOUT 2a : touche M = changer de méthode -----
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        Application* app = (Application*)glfwGetWindowUserPointer(window);
        app->m_methodeBernstein = !app->m_methodeBernstein;
        std::cout << "Methode : "
            << (app->m_methodeBernstein ? "Bernstein (Pascal)" : "De Casteljau")
            << std::endl;
    }
}
// ----- FIN AJOUT 2a -----

int main(void)
{
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