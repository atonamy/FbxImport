/*
Arseniy Kucherenko
*/


#include "minko/Minko.hpp"
#include "minko/MinkoSDL.hpp"
#include "minko/MinkoPNG.hpp"
#include "minko/MinkoASSIMP.hpp"


using namespace minko;
using namespace minko::component;
using namespace minko::math;

//const std::string TEXTURE_FILENAME = "model/box.png";
const std::string EFFECT_FILENAME = "effect/Basic.effect";
const std::string FBX_MODEL_FILENAME = "model/chair.fbx";

int main(int argc, char *argv[])
{
    
    auto canvas = Canvas::create("Loading fbx file");
    auto sceneManager = component::SceneManager::create(canvas);
    
    // Set options for the default loader (the one use to load our file)
    sceneManager->assets()->loader()->options()
    //->registerParser<file::PNGParser>("png")
    ->registerParser<file::FBXParser>("fbx");
    
    sceneManager->assets()->loader()
    ->queue(EFFECT_FILENAME)
    //->queue(TEXTURE_FILENAME)
    ->queue(FBX_MODEL_FILENAME);
    
    auto root = scene::Node::create("root")
    ->addComponent(sceneManager);
    
    auto camera = scene::Node::create("camera")
    ->addComponent(Renderer::create(0x7f7f7fff))
    ->addComponent(Transform::create(math::inverse(math::lookAt(math::vec3(0.f, 0.f, 5.f), math::vec3(), math::vec3(0.f, 1.f, 0.f)))))
    ->addComponent(PerspectiveCamera::create(1.333334f, (float) M_PI * 0.25f, .1f, 1000.f));
    
  
    root->addChild(camera);
    
    // Retrieve our FBX model
    std::shared_ptr<minko::scene::Node> chair = NULL;
   
    auto fileLoaderComplete = sceneManager->assets()->loader()->complete()->connect([&](file::Loader::Ptr loader)
                                                                                    {
                                                                                        chair = sceneManager->assets()->symbol(FBX_MODEL_FILENAME);
                                                                                        //auto chairMaterial = material::BasicMaterial::create();
                                                                                        //chairMaterial->diffuseMap(sceneManager->assets()->texture(TEXTURE_FILENAME));
                                                                                        auto colorMaterial = material::BasicMaterial::create();
                                                                                        colorMaterial->diffuseColor(vec4(0.2f, 0.4f, 0.6f, 0.9f));
                                                                                        
                                                                                        auto surfaceNodeSet = scene::NodeSet::create(chair)
                                                                                        ->descendants(true)
                                                                                        ->where([](scene::Node::Ptr n)
                                                                                                {
                                                                                                    return n->hasComponent<Surface>();
                                                                                                });
                                                                                      
                                                                                        for (auto surfaceNode : surfaceNodeSet->nodes())
                                                                                        {
                                                                                            auto surface = surfaceNode->component<Surface>();
                                                                                            surface->material(colorMaterial);
                                                                                            //surface->material(cubeMaterial);
                                                                                            surface->effect(sceneManager->assets()->effect(EFFECT_FILENAME));
                                                                                        }
                                                                                        
                                                                                        
                                                                                        chair->component<Transform>()->matrix(
                                                                                                                              chair->component<Transform>()->matrix() * math::translate(math::vec3(0, -0.5f, 0))
                                                                                                                              );

                                                                                        chair->component<Transform>()->matrix(chair->component<Transform>()->matrix() * math::scale(math::vec3(0.005f)));
                                                                                        // Add the symbol to the scene
                                                                                        root->addChild(chair);
    
                                                                                    });
    
    float current_x = 0;
    float current_y = 0;
    float previous_x = 0;
    float previous_y = 0;
    auto touchDown = false;
    
    auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr c, float t, float dt)
                                                    {
                                                        if(chair != NULL && touchDown && (current_y != previous_y || current_x != previous_x)) {
                                                            chair->component<Transform>()->matrix(
                                                                                                  chair->component<Transform>()->matrix() * math::rotate((current_x+current_y)/100.f, math::vec3((current_y != previous_y)?1:0, (current_x != previous_x)?1:0, 0))
                                                                                            );
                                                            previous_x = current_x;
                                                            previous_y = current_y;
                                                        }

                                                        sceneManager->nextFrame(t, dt);
                                                    });
    
    
    auto touchMoveSlot = canvas->touch()->touchMove()->connect([&](input::Touch::Ptr t, int a, float x, float y) {
        if(touchDown) {
            current_x = x;
            current_y = y;
        }
    });
    
    auto touchDownSlot = canvas->touch()->touchDown()->connect([&](input::Touch::Ptr t, int a, float x, float y)
                                                               {
                                                                   touchDown = true;
                                                               });
    
    auto touchUpSlot = canvas->touch()->touchUp()->connect([&](input::Touch::Ptr t, int a, float x, float y)
                                                           {
                                                               touchDown = false;
                                                               current_x = 0;
                                                               current_y = 0;
                                                               
                                                           });
    
    
    // Start the scene loading
    sceneManager->assets()->loader()->load();
    
    canvas->run();

    
    return 0;
    
}
