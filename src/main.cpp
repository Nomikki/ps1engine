#include <engine.hpp>
#include <fstream>
#include <iostream>
#include <string>

bool needUpdate = true;

void handleInputs(Engine *engine, Camera *camera, float cameraSpeed,
                  float cameraTurning) {
  if (engine->components.components.empty()) {
    return;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
    engine->components.components[0].transform.rot.y +=
        cameraTurning * engine->getClock();
    engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z);
    needUpdate = true;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
    engine->components.components[0].transform.rot.y -=
        cameraTurning * engine->getClock();
    engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z);
    needUpdate = true;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
    engine->components.components[0].transform.rot.x +=
        cameraTurning * engine->getClock();
    engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z);
    needUpdate = true;
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
    engine->components.components[0].transform.rot.x -=
        cameraTurning * engine->getClock();
    engine->components.components[0].transform.calculateAngles(
        engine->components.components[0].transform.rot.x,
        engine->components.components[0].transform.rot.y,
        engine->components.components[0].transform.rot.z);
    needUpdate = true;
  }
}

bool loadModel(Engine *engine, const std::string &filename) {
  try {
    std::ifstream file(filename);
    if (!file.good()) {
      return false;
    }
    file.close();

    Vec3 position = {0, 0, 0};
    if (!engine->components.createFromFile(filename, -1, position, true)) {
      return false;
    }

    if (engine->components.components.empty()) {
      return false;
    }

    return true;
  } catch (const std::exception &e) {
    return false;
  } catch (...) {
    return false;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }

  Engine *engine = new Engine(60, 4, "PS1 Model Viewer");
  engine->setSort(true);
  engine->setDither(true);
  Camera *camera = new Camera();
  camera->pos = {0, 0, -10};
  camera->vTarget = {0, 0, 0};
  camera->vUp = {0, 1, 0};

  if (!loadModel(engine, argv[1])) {
    delete camera;
    delete engine;
    return 1;
  }

  const float cameraSpeed = 5;
  const float cameraTurning = 1.0;

  while (engine->isOpen()) {
    engine->clear();
    engine->checkEvents();

    handleInputs(engine, camera, cameraSpeed, cameraTurning);
    if (needUpdate) {
      camera->Update(cameraSpeed * engine->getClock());
    }

    engine->calculateTriangles(camera->pos, camera->vTarget, camera->vUp);
    engine->render(0);
    needUpdate = false;
  }

  delete camera;
  delete engine;
  return 0;
}