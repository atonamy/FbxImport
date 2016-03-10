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

typedef std::map<int, float>::iterator it_type;

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
    float currentScale = 0.005f;
   
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

                                                                                        chair->component<Transform>()->matrix(chair->component<Transform>()->matrix() * math::scale(math::vec3(currentScale)));
                                                                                        // Add the symbol to the scene
                                                                                        root->addChild(chair);
    
                                                                                    });
    
   

    
    std::map<int, float> touchY;
    
    auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr c, float t, float dt)
                                                    {
                                                       

                                                        sceneManager->nextFrame(t, dt);
                                                    });
    
    
    auto touchMoveSlot = canvas->touch()->touchMove()->connect([&](input::Touch::Ptr t, int id, float move_x, float move_y) {
        
        if(chair != NULL) {
            
            if(touchY.size() == 1) {
                if(move_x != 0)
                    chair->component<Transform>()->matrix(
                                                    chair->component<Transform>()->matrix() * math::rotate(move_x/100.f, math::vec3(0, 1, 0))
                                            );
        
                if(move_x != 0)
                    chair->component<Transform>()->matrix(
                                                    chair->component<Transform>()->matrix() * math::rotate(move_y/100.f, math::vec3(1, 0, 0))
                                            );
            }
            else if(touchY.size() == 2) {
                if(move_y != 0) {
                    
                    float current_y = 0;
                    float second_y = 0;
                    for(it_type iterator = touchY.begin(); iterator != touchY.end(); iterator++) {
                        if(iterator->first == id)
                            current_y = iterator->second;
                        else
                            second_y = iterator->second;
                    }
                    
                    if((current_y > second_y && move_y > 0) || (current_y < second_y && move_y < 0))
                        currentScale =  1.025f;
                    else if((current_y > second_y && move_y < 0) || (current_y < second_y && move_y > 0))
                        currentScale =  0.975f;
                    else
                        currentScale =  1.f;
                    
                    chair->component<Transform>()->matrix(
                                                          chair->component<Transform>()->matrix() * math::scale(math::vec3(currentScale))
                                                          );
                    
                    LOG_INFO("<id-" << id << ">[state Y] current: " << current_y << ", previous:" << second_y);
                }
            }
            
            
            
            
        }
            
        LOG_INFO("<id-" << id << ">[move] X: " << move_x << ", Y:" << move_y);
        LOG_INFO("<id-" << id << ">[scale] value: " << currentScale);
    });
    
    
    auto touchDownSlot = canvas->touch()->touchDown()->connect([&](input::Touch::Ptr t, int id, float x, float y)
                                                               {
                                                                   touchY[id] = y;
                                                                   LOG_INFO("<id-" << id << ">[touch down] X: " << x << ", Y: " << y);
                                                               });
    
    auto touchUpSlot = canvas->touch()->touchUp()->connect([&](input::Touch::Ptr t, int id, float x, float y)
                                                           {
                                                               touchY.erase(id);
                                                               LOG_INFO("<id-" << id << ">[touch up] X: " << x << ", Y: " << y);
                                                           });
    
    
    // Start the scene loading
    sceneManager->assets()->loader()->load();
    
    canvas->run();

    
    return 0;
    
}
