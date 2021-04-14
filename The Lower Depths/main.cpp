#include "Image.h"
#include "Game.h"
#include "common.h"

#include <GLFW/glfw3.h>


constexpr int ZOOM_COEF = 2;
constexpr GLsizei WINDOW_WIDTH = TILE_SIZE * MAP_WIDTH * ZOOM_COEF;
constexpr GLsizei WINDOW_HEIGHT = TILE_SIZE * MAP_HEIGHT * ZOOM_COEF;

constexpr Pixel BG_COLOR{41, 60, 66, 255};

void glfw_error_callback(int error, const char* description);
void glfw_setup();
GLFWwindow *setup_window();
void gl_setup();
void print_game_info();

void OnKeyboardPressed(GLFWwindow *window, int key, int scancode, int action, int mode);
void OnMouseButtonClicked(GLFWwindow *window, int button, int action, int mods);
void OnMouseMove(GLFWwindow *window, double xpos, double ypos);

void ProcessMovement(Game &game);
void GameUpdate(Game &game);
void GameRender(Game &game);

void GameEffects(const Game &game);

struct InputState {
    bool keys[1024] = {false};
    GLfloat lastX = 400, lastY = 300;
    bool firstMouse = true;
    bool captureMouse = true;
    bool capturedMouseJustNow = false;
} static Input;
std::array<Image, 5> lightnings;

int main() {
    glfw_setup();
    GLFWwindow *window = setup_window();
    gl_setup();
    std::cout << glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR) << std::endl;
    print_game_info();
    Game game;
    glfwSetTime(0);
    while (!glfwWindowShouldClose(window)) {
        GameRender(game);   
        GameEffects(game);
        glfwSwapBuffers(window);
        glfwPollEvents();
        GameUpdate(game);
    }
    glfwTerminate();
    return 0;
}

void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW error with code " << error << " =(" << std::endl;
    std::cerr << description << std::endl;
    // std::cerr << "Bye-bye! (exit)" << std::endl;
    // exit(1);
}

void glfw_setup() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW =(" << std::endl;
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

GLFWwindow *setup_window() {
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
        "На дне", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window =(" << std::endl;
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, OnKeyboardPressed);
    glfwSetCursorPosCallback(window, OnMouseMove);
    glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
    return window;
}

void gl_setup() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        exit(1);
    }
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(41 / 255.0f, 60 / 255.0f, 66 / 255.0f, 1.0f);
    glfwSwapInterval(1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glRasterPos2f(-1, 1);
    glPixelZoom(ZOOM_COEF, -ZOOM_COEF);
}

void print_game_info() {
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
              << std::endl;
}

void OnKeyboardPressed(GLFWwindow *window, int key, int scancode, int action, int mode) {
    switch (key) {
        case GLFW_KEY_ESCAPE:
            if (action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GL_TRUE);
                std::cout << "Window is closed!" << std::endl;
            }
            break;
        case GLFW_KEY_1:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case GLFW_KEY_2:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        default:
            if (action == GLFW_PRESS)
                Input.keys[key] = true;
            else if (action == GLFW_RELEASE)
                Input.keys[key] = false;
    }
}

void OnMouseButtonClicked(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        Input.captureMouse = !Input.captureMouse;
    if (Input.captureMouse) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        Input.capturedMouseJustNow = true;
    } else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void OnMouseMove(GLFWwindow *window, double xpos, double ypos) {
    if (Input.firstMouse) {
        Input.lastX = float(xpos);
        Input.lastY = float(ypos);
        Input.firstMouse = false;
    }

    GLfloat xoffset = float(xpos) - Input.lastX;
    GLfloat yoffset = Input.lastY - float(ypos);

    Input.lastX = float(xpos);
    Input.lastY = float(ypos);
}

void ProcessMovement(Game &game) {
    if (Input.keys[GLFW_KEY_Q]) {
        game.ActivatePearl();
    }
    if (Input.keys[GLFW_KEY_W]) {
        game.Move(Direction::UP);
    } else if (Input.keys[GLFW_KEY_S]) {
        game.Move(Direction::DOWN);
    }
    if (Input.keys[GLFW_KEY_A]) {
        game.Move(Direction::LEFT);
    } else if (Input.keys[GLFW_KEY_D]) {
        game.Move(Direction::RIGHT);
    }
}

void GameUpdate(Game &game) {
    game.UpdTime(glfwGetTime());
    if (game.State() == GameState::PLAY) {
        game.MoveGuards();
        ProcessMovement(game);
        game.RoomChangeCheck();
    }
}

void GameRender(Game &game) {
    glClear(GL_COLOR_BUFFER_BIT);
    for (auto [pos, obj]: game.DrawList()) {
        glWindowPos2i(ZOOM_COEF * pos.x, WINDOW_HEIGHT - ZOOM_COEF * pos.y);
        if (obj.width() > TILE_SIZE * MAP_WIDTH) { // effect
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        } else {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        glDrawPixels(obj.width(), obj.height(), GL_RGBA, GL_UNSIGNED_BYTE, obj.data());
        
    }
}

void GameEffects(const Game &game) {
    GLfloat room_change_fade = std::min(static_cast<GLfloat>(game.RoomFade()), static_cast<GLfloat>(1));
    if (room_change_fade > 0) {
        glWindowPos2i(0, WINDOW_HEIGHT);
        glBlendColor(0, 0, 0, room_change_fade);
        glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        static Image img(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, BG_COLOR);
        glDrawPixels(img.width(), img.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.data());
    }
}