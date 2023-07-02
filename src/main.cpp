#include <engine.hpp>

void handleInputs(Engine *engine, Camera *camera, float cameraSpeed, float cameraTurning)
{
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    camera->pos = Vector_Add(camera->pos, camera->vRight);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    camera->pos = Vector_Sub(camera->pos, camera->vRight);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    camera->pos.y -= cameraSpeed * engine->getClock();

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
    camera->pos.y += cameraSpeed * engine->getClock();

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    camera->pos = Vector_Add(camera->pos, camera->vForward);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    camera->pos = Vector_Sub(camera->pos, camera->vForward);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    camera->yaw += cameraTurning * engine->getClock();

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    camera->yaw -= cameraTurning * engine->getClock();
}

int main()
{
  // init
  Engine *engine = new Engine(0, 4, "PS1 clone");
  engine->setSort(true);

  Camera *camera = new Camera();

  for (int k = 0; k < 10; k++)
    for (int j = 0; j < 10; j++)
      engine->components.createFromFile("spyro/spyro.obj", Vec3{-10.0f + (k * 2), 0, 5.0f + (j * 4)});

  const float cameraSpeed = 5;
  const float cameraTurning = 1.0;

  while (engine->isOpen())
  {
    engine->checkEvents();

    handleInputs(engine, camera, cameraSpeed, cameraTurning);

    camera->Update(cameraSpeed * engine->getClock());
    engine->calculateTriangles(camera->pos, camera->vTarget, camera->vUp);
    engine->renderAll();
  }

  delete engine;
}
