#include <renderer.hpp>

void handleInputs(Renderer *renderer, Camera *camera, float cameraSpeed, float cameraTurning)
{
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    camera->pos = Vector_Add(camera->pos, camera->vRight);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    camera->pos = Vector_Sub(camera->pos, camera->vRight);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
    camera->pos.y -= cameraSpeed * renderer->getClock();

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
    camera->pos.y += cameraSpeed * renderer->getClock();

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    camera->pos = Vector_Add(camera->pos, camera->vForward);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    camera->pos = Vector_Sub(camera->pos, camera->vForward);

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    camera->yaw += cameraTurning * renderer->getClock();

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    camera->yaw -= cameraTurning * renderer->getClock();
}

int main()
{
  // init
  Renderer *renderer = new Renderer(0, 4, "PS1 clone");
  renderer->setSort(true);

  Camera *camera = new Camera();

  for (int k = 0; k < 20; k++)
    for (int j = 0; j < 20; j++)
      renderer->components.createFromFile("spyro/spyro.obj", Vec3{-10.0f + (k * 2), 0, 5.0f + (j * 4)});

  const float cameraSpeed = 5;
  const float cameraTurning = 1.0;

  while (renderer->isOpen())
  {
    renderer->checkEvents();

    handleInputs(renderer, camera, cameraSpeed, cameraTurning);

    camera->Update(cameraSpeed * renderer->getClock());
    renderer->calculateTriangles(camera->pos, camera->vTarget, camera->vUp);
    renderer->renderAll();
  }

  delete renderer;
}
