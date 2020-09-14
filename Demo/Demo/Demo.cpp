#include <Engine/EngineManager.h>
using namespace Demo; //convenience
VimguiCore* guiCore;
Vimgui* gui;
class Application
{
public:
    DemoWindow mainWindow;
    EngineManager* em; //Handle to the EngineManager
    Application();
    ~Application();
    void init();
    void main();
};

Application* app;
void keyboardCallback(uint32_t action, uint32_t key); //Called upon keyboard button event
void mouseMoveCallback(int32_t xpos, int32_t ypos); //Called upon mouse move event
void mouseKeyCallback(uint32_t action, uint32_t key); //Called upon mouse button event
void mouseScrollCallback(int32_t delta); //Called upon mouse scroll event
void t1sCallback(); //Called every second

Application::Application()
{

}

Application::~Application()
{
    delete em;
    em = 0;
}

void Application::init()
{
    em = new EngineManager();
}

void Application::main()
{
    mainWindow = em->windowManager->createStandardWindow(1200, 800, "DemoEngine");
    em->vulkanManager->init(mainWindow.getHWND());
    em->vulkanManager->setClearColor(0.0f, 0.0f, 0.0f);
    em->enableStatisticWindowTitle(&mainWindow);
    em->set1SecondCallback(t1sCallback);
    em->inputManager->setKeyCallback(keyboardCallback);
    em->inputManager->setMouseKeyCallback(mouseKeyCallback);
    em->inputManager->setMouseMoveCallback(mouseMoveCallback);
    em->inputManager->setMouseScrollCallback(mouseScrollCallback);
    em->inputManager->mouse->setScrollStep(20);
    em->vulkanManager->vulkanUIManager->buildCommandBuffer();
    guiCore = app->em->vulkanManager->vulkanUIManager->vimgui->vimguiCore;
    gui = app->em->vulkanManager->vulkanUIManager->vimgui;
    while (!em->windowManager->windowShouldClose(mainWindow))
    {
        em->main();
    }
}


int main()
{
    app = new Application();
    app->init();
    app->main();
    delete app;
    return 0;
}

void t1sCallback()
{

}

void keyboardCallback(uint32_t action, uint32_t key)
{
    app->em->vulkanManager->vulkanUIManager->keyboardEvent(action, key);
    if (action == KEY_ACTION_DOWN && key == KEY_ESCAPE)
    {
        PostQuitMessage(0);
        std::cout << "" << std::endl;
        app->em->windowManager->exitFlag = true;
    }

    if (action == KEY_ACTION_DOWN && key == KEY_F5)
    {
        gui->regenerateUserInterface();
    }

}

void mouseMoveCallback(int32_t xpos, int32_t ypos)
{
    //app->em->vulkanManager->vulkanUIManager->mouseMoveEvent(xpos, ypos);
    if (app->em->inputManager->mouse->isLButtonDown())
    {

    }
    if (app->em->inputManager->mouse->isRButtonDown())
    {
        //app->em->vulkanManager->vulkanUIManager->vimgui->removeDistanceFieldText(app->em->inputManager->mouse->getMouseX(), app->em->inputManager->mouse->getMouseY());
        //app->em->vulkanManager->vulkanUIManager->buildCommandBuffer();
    }
    if (app->em->inputManager->mouse->isMButtonDown())
    {
       gui->movePanel(gui->findPanelheader(app->em->inputManager->mouse->getMouseX(), app->em->inputManager->mouse->getMouseY() ), app->em->inputManager->mouse->getDeltaStepX(), app->em->inputManager->mouse->getDeltaStepY() );
        app->em->vulkanManager->vulkanUIManager->buildCommandBuffer();
    }

}

void mouseKeyCallback(uint32_t key, uint32_t additional)
{
    if (key == MOUSE_LBUTTONDOWN)
    {
        Panel tp;
        tp.panelBody = true;
        tp.panelHeader = true;
        tp.panelOutline = true;
        tp.width = 400;
        tp.height = 800;
        tp.posx = app->em->inputManager->mouse->getMouseX();
        tp.posy = app->em->inputManager->mouse->getMouseY();
        tp.panelTitle = true;
        tp.panelTitleString = L"testpanel";
        gui->genPanel(tp);
        app->em->vulkanManager->vulkanUIManager->buildCommandBuffer();
    }
    if (key == MOUSE_RBUTTONDOWN)
    {
        gui->destroyPanel(gui->findPanelheader(app->em->inputManager->mouse->getMouseX(), app->em->inputManager->mouse->getMouseY()) );
        app->em->vulkanManager->vulkanUIManager->buildCommandBuffer();
        app->em->vulkanManager->vulkanUIManager->mouseKeyEvent(key, additional);
    }

    if (key == MOUSE_MBUTTONDOWN)
    {

    }


}

void mouseScrollCallback(int32_t delta)
{
    //obs, scrollstep.
    app->em->vulkanManager->vulkanUIManager->mouseScrollEvent(delta);
}






