#include <engine.hpp>
#include <iostream>
#include <string>

bool needUpdate = true;

void handleInputs(Engine *engine, Camera *camera, float cameraSpeed, float cameraTurning)
{
  if (!engine->components.components.empty()) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
      engine->components.components[0].transform.rot.y += cameraTurning * engine->getClock();
      engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z
      );
      needUpdate = true;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
      engine->components.components[0].transform.rot.y -= cameraTurning * engine->getClock();
      engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z
      );
      needUpdate = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
      engine->components.components[0].transform.rot.x += cameraTurning * engine->getClock();
      engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z
      );
      needUpdate = true;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
      engine->components.components[0].transform.rot.x -= cameraTurning * engine->getClock();
      engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z
      );
      needUpdate = true;
    }
  }
}

bool loadModel(Engine* engine, const std::string& filename, const Vec3& position = {0, 0, 0})
{
    try {
        engine->components.createFromFile(filename, -1, position);
        std::cout << "Successfully loaded model: " << filename << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading model " << filename << ": " << e.what() << std::endl;
        return false;
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Error: Please specify exactly one model file" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <model.obj>" << std::endl;
        return 1;
    }

    Engine* engine = new Engine(60, 4, "PS1 Model Viewer");
    engine->setSort(false);
    engine->setDither(true);

    Camera* camera = new Camera();
    camera->pos = {0, 0, -5};
    camera->vTarget = {0, 0, 0};
    camera->vUp = {0, 1, 0};

    if (!loadModel(engine, argv[1])) {
        std::cerr << "Error: Failed to load model" << std::endl;
        delete camera;
        delete engine;
        return 1;
    }

    const float cameraSpeed = 5;
    const float cameraTurning = 1.0;

    while (engine->isOpen())
    {
        engine->checkEvents();

        handleInputs(engine, camera, cameraSpeed, cameraTurning);
        if (needUpdate)
        {
            camera->Update(cameraSpeed * engine->getClock());
        }

        engine->calculateTriangles(camera->pos, camera->vTarget, camera->vUp);
        engine->renderAll();
        needUpdate = false;
    }

    delete camera;
    delete engine;
    return 0;
}
